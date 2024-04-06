#pragma once

namespace chowdsp
{
/** Filter type options for Chebyshev Filters */
enum class ChebyshevFilterType
{
    Lowpass,
    Highpass
};

/**
 * A variable-order Type II Chebyshev filter.
 *
 * @tparam order The filter order (must be even)
 * @tparam type  The filter type
 * @tparam stopBandAttenuationDB    The attenuation in the stop-band of the filter (should be given as a positive number)
 * @tparam useNaturalCutoff         True if the cutoff frequency should refer to the -3dB point. Otherwise the cutoff frequency will refer to the start of the stop-band region.
 * @tparam FloatType    The floating point type to use
 */
template <int order, ChebyshevFilterType type = ChebyshevFilterType::Lowpass, int stopBandAttenuationDB = 60, bool useNaturalCutoff = true, typename FloatType = float>
class ChebyshevIIFilter : public SOSFilter<order, FloatType>
{
    static constexpr auto NFilters = (size_t) order / 2;

public:
    using NumericType = SampleTypeHelpers::NumericType<FloatType>;
    static constexpr bool HasQParameter = true;
    static constexpr bool HasGainParameter = false;

    ChebyshevIIFilter()
    {
        calcConstants();
    }

    /**
     * Calculates the coefficients for a higher-order Chebyshev filter.
     *
     * If `useNaturalCutoff` is true, the cutoff frequency `fc` refers
     * to the -3dB point.
     *
     * Otherwise `fc` refers to the frequency at which the stop-band
     * starts (i.e. at any frequency above fc, the output will be
     * attenuated by at least stopBandAttenuationDB).
     */
    void calcCoefs (FloatType fc, FloatType qVal, NumericType fs)
    {
        juce::ignoreUnused (qVal);

        FloatType bCoefs[3], bOppCoefs[3], aCoefs[3];
        auto calcBaseCoefficients = [&] (FloatType stageFreqOff, FloatType stageQ)
        {
            switch (type)
            {
                case ChebyshevFilterType::Lowpass:
                    CoefficientCalculators::calcSecondOrderLPF<FloatType, NumericType, false> (bCoefs, aCoefs, fc * stageFreqOff, stageQ, fs, fc);
                    CoefficientCalculators::calcSecondOrderHPF<FloatType, NumericType, false> (bOppCoefs, aCoefs, fc * stageFreqOff, stageQ, fs, fc);
                    break;
                case ChebyshevFilterType::Highpass:
                    CoefficientCalculators::calcSecondOrderLPF<FloatType, NumericType, false> (bOppCoefs, aCoefs, fc / stageFreqOff, stageQ, fs, fc);
                    CoefficientCalculators::calcSecondOrderHPF<FloatType, NumericType, false> (bCoefs, aCoefs, fc / stageFreqOff, stageQ, fs, fc);
                    break;
            }
        };

        auto calcCoefsForQ = [&] (FloatType stageFreqOff, FloatType stageQ, FloatType stageLPGain, size_t stageOrder)
        {
            calcBaseCoefficients (stageFreqOff, stageQ);

            for (size_t i = 0; i < 3; ++i)
                bCoefs[i] = bOppCoefs[i] + stageLPGain * bCoefs[i];

            if (stageOrder == 0)
            {
                for (auto& b : bCoefs)
                    b *= juce::Decibels::decibelsToGain ((NumericType) -stopBandAttenuationDB);
            }

            this->secondOrderSections[stageOrder].setCoefs (bCoefs, aCoefs);
        };

        calcCoefsForQ (freqOffsets[0], qVals[0] * qVal * juce::MathConstants<NumericType>::sqrt2, lpGains[0], 0);
        for (size_t i = 1; i < NFilters; ++i)
            calcCoefsForQ (freqOffsets[i], qVals[i], lpGains[i], i);
    }

private:
    using Complex = std::complex<NumericType>;
    using PZSet = std::array<Complex, NFilters>;

    /** mostly a re-implementation of scipy.signal.cheb2ap */
    static void cheb2ap (PZSet& poles, PZSet& zeros)
    {
        static constexpr auto rs = (NumericType) stopBandAttenuationDB;
        const auto de = (NumericType) 1 / std::sqrt (std::pow ((NumericType) 10, (NumericType) 0.1 * rs) - (NumericType) 1);
        const auto mu = std::asinh ((NumericType) 1 / de) / (NumericType) order;

        const auto sinh_mu = std::sinh (mu);
        const auto cosh_mu = std::cosh (mu);
        const auto fn = juce::MathConstants<NumericType>::pi / NumericType (2 * order);

        size_t k = 1;
        int i = order / 2;
        while (i - 1 >= 0)
        {
            const auto a = sinh_mu * std::cos (NumericType ((int) k - order) * fn);
            const auto b = cosh_mu * std::sin (NumericType ((int) k - order) * fn);
            const auto d2 = a * a + b * b;
            const auto im = (NumericType) 1 / std::cos ((NumericType) k * fn);

            poles[(k - 1) / 2] = std::complex { a / d2, b / d2 };
            zeros[(k - 1) / 2] = std::complex { (NumericType) 0, im };

            --i;
            k += 2;
        }
    }

    [[maybe_unused]] static NumericType computeTransitionBandShift()
    {
        // so the Chebyshev filter equations do everything in reference to the
        // start of the stop-band region. To get the "natural" cutoff frequency (-3dB point),
        // we need to divide by the width of the transition band. To figure that out, we
        // reverse engineer the cheb2ord equation (https://github.com/scipy/scipy/blob/ba77f000ded56d07c9f52a154fbded87f2e419b2/scipy/signal/_filter_design.py#L3948)

        const auto Gpass = std::pow ((NumericType) 10, (NumericType) 0.1 * std::abs ((NumericType) 3));
        const auto Gstop = std::pow ((NumericType) 10, (NumericType) 0.1 * std::abs ((NumericType) stopBandAttenuationDB));
        const auto sqrtG = std::sqrt ((Gstop - 1) / (Gpass - 1));
        return std::cosh (std::acosh (sqrtG) / (NumericType) order); // ratio of stop-band frequency to pass-band frequency
    }

    void calcConstants()
    {
        PZSet poles, zeros;
        cheb2ap (poles, zeros);
        const auto transitionBandShiftFactor = useNaturalCutoff ? computeTransitionBandShift() : (NumericType) 1;

        for (size_t i = 0; i < NFilters; ++i)
        {
            using Power::ipow;
            const auto p_norm = std::sqrt (ipow<2> (poles[i].real()) + ipow<2> (poles[i].imag()));
            freqOffsets[i] = p_norm * transitionBandShiftFactor;
            qVals[i] = p_norm / ((NumericType) 2 * std::abs (poles[i].real()));
            lpGains[i] = ipow<2> (zeros[i].imag()) / ipow<2> (p_norm);
        }
    }

    std::array<NumericType, NFilters> freqOffsets {};
    std::array<NumericType, NFilters> qVals {};
    std::array<NumericType, NFilters> lpGains {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChebyshevIIFilter)
};
} // namespace chowdsp
