#pragma once

namespace chowdsp
{
/** Filter type options for Chebyshev Filters */
enum class ChebyshevFilterType
{
    Lowpass,
    Highpass
};

/** A variable-order Type II Chebyshev filter */
template <int order, ChebyshevFilterType type = ChebyshevFilterType::Lowpass, int stopBandAttenuationDB = 60, typename FloatType = float>
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
     * Calculates the coefficients for a higher-order Butterworth filter.
     *
     * Note that the cutoff frequency fc refers to the frequency at which
     * the stop-band starts (i.e. at any frequency above fc, the output
     * will be attenuated by at least stopBandAttenuationDB.
     */
    void calcCoefs (FloatType fc, FloatType qVal, NumericType fs)
    {
        juce::ignoreUnused (qVal);

        FloatType bCoefs[3], bOppCoefs[3], aCoefs[3];
        auto calcCoefsForQ = [&] (FloatType stageFreqOff, FloatType stageQ, FloatType stageLPGain, size_t stageOrder) {
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
        for (int i = order / 2; --i >= 0; k += 2)
        {
            const auto a = sinh_mu * std::cos (NumericType ((int) k - order) * fn);
            const auto b = cosh_mu * std::sin (NumericType ((int) k - order) * fn);
            const auto d2 = a * a + b * b;
            const auto im = (NumericType) 1 / std::cos ((NumericType) k * fn);

            poles[(k - 1) / 2] = std::complex { a / d2, b / d2 };
            zeros[(k - 1) / 2] = std::complex { (NumericType) 0, im };
        }
    }

    void calcConstants()
    {
        PZSet poles, zeros;
        cheb2ap (poles, zeros);

        for (size_t i = 0; i < NFilters; ++i)
        {
            using Power::ipow;

            const auto p_norm = std::sqrt (ipow<2> (poles[i].real()) + ipow<2> (poles[i].imag()));
            freqOffsets[i] = p_norm;
            qVals[i] = p_norm / ((NumericType) 2 * poles[i].real());
            lpGains[i] = ipow<2> (zeros[i].imag()) / ipow<2> (p_norm);
        }
    }

    std::array<NumericType, NFilters> freqOffsets;
    std::array<NumericType, NFilters> qVals;
    std::array<NumericType, NFilters> lpGains;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChebyshevIIFilter)
};
} // namespace chowdsp
