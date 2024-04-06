#pragma once

namespace chowdsp
{
/** Filter type options for Elliptic Filters */
enum class EllipticFilterType
{
    Lowpass,
    Highpass
};

/**
 * A variable-order Elliptic filter.
 *
 * @tparam order The filter order (must be even)
 * @tparam type  The filter type
 * @tparam stopBandAttenuationDB    The attenuation in the stop-band of the filter (should be given as a positive number)
 * @tparam passbandRippleDB         A chowdsp::Ratio describing the amount of ripple allowed in the pass-band of the filter (in Decibels)
 * @tparam FloatType    The floating point type to use
 */
template <int order, EllipticFilterType type = EllipticFilterType::Lowpass, int stopBandAttenuationDB = 60, typename passbandRippleDB = Ratio<1, 10>, typename FloatType = float>
class EllipticFilter : public SOSFilter<order, FloatType>
{
    static constexpr auto NFilters = (size_t) order / 2;

public:
    using NumericType = SampleTypeHelpers::NumericType<FloatType>;
    static constexpr bool HasQParameter = true;
    static constexpr bool HasGainParameter = false;

    EllipticFilter()
    {
        calcConstants();
    }

    /**
     * Calculates the coefficients for a higher-order Elliptic filter.
     *
     * Note that the cutoff frequency fc refers to the frequency at which
     * the transition-band starts.
     */
    void calcCoefs (FloatType fc, FloatType qVal, NumericType fs)
    {
        auto calcCoefsForQ = [&] (FloatType stageFreqOff, FloatType stageQ, FloatType stageLPGain, size_t stageOrder)
        {
            FloatType bCoefs[3], bOppCoefs[3], aCoefs[3];
            switch (type)
            {
                case EllipticFilterType::Lowpass:
                    CoefficientCalculators::calcSecondOrderLPF<FloatType, NumericType, false> (bCoefs, aCoefs, fc * stageFreqOff, stageQ, fs, fc);
                    CoefficientCalculators::calcSecondOrderHPF<FloatType, NumericType, false> (bOppCoefs, aCoefs, fc * stageFreqOff, stageQ, fs, fc);
                    break;
                case EllipticFilterType::Highpass:
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

        for (size_t i = 0; i < NFilters - 1; ++i)
            calcCoefsForQ (freqOffsets[i], qVals[i], lpGains[i], i);
        calcCoefsForQ (freqOffsets[NFilters - 1], qVals[NFilters - 1] * qVal * juce::MathConstants<NumericType>::sqrt2, lpGains[NFilters - 1], NFilters - 1);
    }

private:
    using Complex = std::complex<NumericType>;
    using PZSet = std::array<Complex, NFilters>;

    // approximation to complete elliptic integral of the first kind.
    // fast convergence, peak error less than 2e-16.
    static double ellipticK (double k)
    {
        double m = k * k;
        double a = 1;
        double b = std::sqrt (1 - m);
        double c = a - b;
        double co;
        do
        {
            co = c;
            c = (a - b) / 2;
            double ao = (a + b) / 2;
            b = std::sqrt (a * b);
            a = ao;
        } while (c < co);

        return juce::MathConstants<double>::pi / (a + a);
    }

    /**
     * Solve degree equation using nomes
     * Given n, m1, solve
     * n * K(m) / K'(m) = K1(m1) / K1'(m1)
     * for m
     */
    static double ellipdeg (double n, double m1)
    {
        constexpr int ELLIPDEG_MMAX = 7;

        const auto K1 = ellipticK (std::sqrt (m1));
        const auto K1p = ellipticK (std::sqrt (1.0 - m1));

        const auto q1 = std::exp (-juce::MathConstants<double>::pi * K1p / K1);
        const auto q = std::pow (q1, 1.0 / n);

        double num = 0.0, den = 0.0;
        for (int i = 0; i < ELLIPDEG_MMAX + 1; ++i)
        {
            num += std::pow (q, double (i * (i + 1)));
            den += std::pow (q, double (i + 1) * double (i + 1));
        }

        return 16.0 * q * std::pow (num / (1.0 + 2.0 * den), 4.0);
    }

    /**
     * Inverse Jacobian elliptic sn
     *  Solve for z in w = sn(z, m)
     *  Parameters
     *  ----------
     *  w - complex scalar
     *      argument
     *  m - scalar
     *      modulus; in interval [0, 1]
     */
    static std::complex<double> arc_jac_sn (std::complex<double> w, double m)
    {
        // Maximum number of iterations in Landen transformation recursion
        // sequence.  10 is conservative; unit tests pass with 4, Orfanidis
        // (see _arc_jac_cn [1]) suggests 5.
        constexpr int ARC_JAC_SN_MAXITER = 10;

        auto complement = [] (auto kx)
        {
            using T_kx = decltype (kx);
            return std::pow ((T_kx (1.0) - kx) * (T_kx (1.0) + kx), 0.5);
        };

        const auto k = std::sqrt (m);

        jassert (k <= 1.0); // k is ill-formed
        if (juce::exactlyEqual (k, 1.0))
            return std::atanh (w);

        std::vector<double> ks { k };
        int nIter = 0;
        while (! juce::exactlyEqual (ks.back(), 0.0))
        {
            const auto k_ = ks.back();
            const auto k_p = complement (k_);
            ks.push_back ((1.0 - k_p) / (1.0 + k_p));
            nIter++;

            if (nIter > ARC_JAC_SN_MAXITER)
            {
                jassertfalse; // Landen transformation not converging
                break;
            }
        }

        const auto K = std::accumulate (ks.begin() + 1, ks.end(), 1.0, [] (double prev, double next)
                                        { return prev * (1.0 + next); })
                       * juce::MathConstants<double>::pi * 0.5;

        std::vector<std::complex<double>> wns { w };
        for (size_t i = 0; i < ks.size() - 1; ++i)
        {
            const auto kn = ks[i];
            const auto knext = ks[i + 1];

            const auto wn = wns.back();
            const auto wnext = ((double) 2 * wn / ((1.0 + knext) * (std::complex<double> (1.0) + complement (kn * wn))));
            wns.push_back (wnext);
        }

        const auto u = 2.0 / juce::MathConstants<double>::pi * std::asin (wns.back());

        return K * u;
    }

    /**
     * Real inverse Jacobian sc, with complementary modulus
     * Solve for z in w = sc(z, 1-m)
     * w - real scalar
     * m - modulus
     * From [1], sc(z, m) = -i * sn(i * z, 1 - m)
     */
    static double arc_jac_sc1 (double w, double m)
    {
        const auto zcomplex = arc_jac_sn (std::complex<double> { 0.0, w }, m);
        jassert (zcomplex.real() <= 1e-14); // ill-formed result
        return zcomplex.imag();
    }

    /** mostly a re-implementation of scipy.signal.cheb2ap */
    static void ellipap (PZSet& poles, PZSet& zeros)
    {
        auto pow10m1 = [] (double x)
        {
            return std::exp (std::log (10.0) * x) - 1.0;
        };

        static constexpr auto rp = passbandRippleDB::template value<double>;
        const auto eps_sq = pow10m1 (0.1 * rp);

        const auto eps = std::sqrt (eps_sq);
        const auto ck1_sq = eps_sq / pow10m1 (0.1 * (double) stopBandAttenuationDB);
        jassert (! juce::exactlyEqual (ck1_sq, 0.0)); // "Cannot design a filter with given rp and rs specifications."

        const auto val0 = ellipticK (std::sqrt (ck1_sq));
        const auto m = ellipdeg ((double) order, ck1_sq);
        const auto capk = ellipticK (std::sqrt (m));

        std::array<double, NFilters> j {};
        for (size_t i = 0; i < j.size(); ++i) // NOSONAR (this loop is simple enough)
            j[i] = double (i * 2 + 1);

        std::array<double, NFilters> s {}, c {}, d {};

        for (size_t i = 0; i < j.size(); ++i)
        {
            std::tie (s[i], c[i], d[i]) = jacobi::jacobi_elliptic (j[i] * capk / (double) order, m);
            zeros[i] = std::complex { (NumericType) 0, NumericType (1.0 / (sqrt (m) * s[i])) };
        }

        const auto r = arc_jac_sc1 (1.0 / eps, ck1_sq);
        const auto v0 = capk * r / ((double) order * val0);

        auto [sv, cv, dv] = jacobi::jacobi_elliptic (v0, 1.0 - m);
        for (size_t i = 0; i < j.size(); ++i)
        {
            const auto den = (d[i] * sv) * (d[i] * sv) - 1.0;
            poles[i] = std::complex { NumericType (c[i] * d[i] * sv * cv), NumericType (s[i] * dv) } / (NumericType) den;
        }
    }

    void calcConstants()
    {
        PZSet poles, zeros;
        ellipap (poles, zeros);

        for (size_t i = 0; i < NFilters; ++i)
        {
            using Power::ipow;

            const auto p_norm = std::sqrt (ipow<2> (poles[i].real()) + ipow<2> (poles[i].imag()));
            freqOffsets[i] = p_norm;
            qVals[i] = p_norm / ((NumericType) 2 * std::abs (poles[i].real()));
            lpGains[i] = ipow<2> (zeros[i].imag()) / ipow<2> (p_norm);
        }
    }

    std::array<NumericType, NFilters> freqOffsets {};
    std::array<NumericType, NFilters> qVals {};
    std::array<NumericType, NFilters> lpGains {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EllipticFilter)
};
} // namespace chowdsp
