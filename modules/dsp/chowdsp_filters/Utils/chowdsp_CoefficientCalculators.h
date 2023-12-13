#pragma once

namespace chowdsp
{
/** Methods for computing coefficients for canonical filter forms */
namespace CoefficientCalculators
{
    /** Q value to use for maximally flat passband gain */
    template <typename T>
    constexpr auto butterworthQ = (T) 1 / juce::MathConstants<T>::sqrt2;

    /**
     * Calculates the filter coefficients for a given cutoff frequency and sample rate.
     * The analog prototype transfer function is: \f$ H(s) = \frac{1}{s + 1} \f$
     */
    template <typename T, typename NumericType>
    void calcFirstOrderLPF (T (&b)[2], T (&a)[2], T fc, NumericType fs)
    {
        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;
        const auto K = ConformalMaps::computeKValueAngular (wc, fs);
        ConformalMaps::Transform<T, 1>::bilinear (b, a, { (T) 0, (T) 1 }, { (T) 1 / wc, (T) 1 }, K);
    }

    /**
     * Calculates the filter coefficients for a given cutoff frequency and sample rate.
     * The analog prototype transfer function is: \f$ H(s) = \frac{s}{s + 1} \f$
     */
    template <typename T, typename NumericType>
    void calcFirstOrderHPF (T (&b)[2], T (&a)[2], T fc, NumericType fs)
    {
        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;
        const auto K = ConformalMaps::computeKValueAngular (wc, fs);
        const auto wc_recip = (T) 1 / wc;
        ConformalMaps::Transform<T, 1>::bilinear (b, a, { wc_recip, (T) 0 }, { wc_recip, (T) 1 }, K);
    }

    /** Calculates the coefficients for the filter.
     *
     * @param b: the filter feed-forward coefficients
     * @param a: the filter feed-back coefficients
     * @param lowGain: the gain of the filter at low frequencies
     * @param highGain: the gain of the filter at high frequencies
     * @param fc: the transition frequency of the filter
     * @param fs: the sample rate for the filter
     *
     * For information on the filter coefficient derivation,
     * see Abel and Berners dsp4dae, pg. 249
     */
    template <typename T, typename NumericType>
    void calcFirstOrderShelf (T (&b)[2], T (&a)[2], T lowGain, T highGain, T fc, NumericType fs)
    {
        // reduce to simple gain element
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wfloat-equal")
        if (SIMDUtils::all (lowGain == highGain))
        {
            b[0] = lowGain;
            b[1] = (T) 0;
            a[0] = (T) 1;
            a[1] = (T) 0;
            return;
        }
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        CHOWDSP_USING_XSIMD_STD (sqrt);
        CHOWDSP_USING_XSIMD_STD (tan);

        const auto rho_recip = (T) 1 / sqrt (highGain / lowGain);
        const auto K = (T) 1 / tan (juce::MathConstants<NumericType>::pi * fc / fs);

        ConformalMaps::Transform<T, 1>::bilinear (b, a, { highGain * rho_recip, lowGain }, { rho_recip, (T) 1 }, K);
    }

    /** Type to specify the method to use for calculating the filter coefficients */
    enum class CoefficientCalculationMode
    {
        Standard, /**< Standard coefficient calculation based on the bilinear transform */
        Decramped, /**< Decramped coefficient calculation using <a href="https://www.vicanek.de/articles/BiquadFits.pdf">Martin Vicanek's method</a>. */
    };

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, and sample rate.
     * The analog prototype transfer function is: \f$ H(s) = \frac{1}{s^2 + s/Q + 1} \f$
     */
    template <typename T, typename NumericType, bool MatchCutoff = true, CoefficientCalculationMode mode = CoefficientCalculationMode::Standard>
    void calcSecondOrderLPF (T (&b)[3], T (&a)[3], T fc, T qVal, NumericType fs, T matchedFc = (T) -1)
    {
        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;

        T matchedWc;
        if constexpr (MatchCutoff)
        {
            juce::ignoreUnused (matchedFc);
            matchedWc = wc;
        }
        else
        {
            matchedWc = matchedFc > (T) 0 ? (juce::MathConstants<NumericType>::twoPi * matchedFc) : wc;
        }

        if constexpr (mode == CoefficientCalculationMode::Standard)
        {
            const auto K = ConformalMaps::computeKValueAngular (matchedWc, fs);
            auto kSqTerm = (T) 1 / (wc * wc);
            auto kTerm = (T) 1 / (qVal * wc);
            ConformalMaps::Transform<T, 2>::bilinear (b, a, { (T) 0, (T) 0, (T) 1 }, { kSqTerm, kTerm, (T) 1 }, K);
        }
        else if constexpr (mode == CoefficientCalculationMode::Decramped)
        {
            qVal = VicanekHelpers::clampQVicanek (qVal);
            if (SIMDUtils::any (fc < (T) 1000))
            {
                calcSecondOrderLPF<T, NumericType, MatchCutoff, CoefficientCalculationMode::Standard> (b, a, fc, qVal, fs, matchedFc);
                return;
            }

            const auto [p0, p1, p2, A0, A1, A2] = VicanekHelpers::computeVicanekPolesAngular (matchedWc, qVal, fs, a);
            const auto R1 = (A0 * p0 + A1 * p1 + A2 * p2) * Power::ipow<2> (qVal);
            const auto B0 = A0;
            const auto B1 = (R1 - B0 * p0) / p1;

            CHOWDSP_USING_XSIMD_STD (sqrt);
            const auto sqrtB0 = sqrt (B0);
            const auto sqrtB1 = sqrt (B1);
            b[0] = (T) 0.5 * (sqrtB0 + sqrtB1);
            b[1] = sqrtB0 - b[0];
            b[2] = (T) 0;
        }
    }

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, and sample rate.
     * The analog prototype transfer function is: \f$ H(s) = \frac{s^2}{s^2 + s/Q + 1} \f$
     */
    template <typename T, typename NumericType, bool MatchCutoff = true, CoefficientCalculationMode mode = CoefficientCalculationMode::Standard>
    void calcSecondOrderHPF (T (&b)[3], T (&a)[3], T fc, T qVal, NumericType fs, T matchedFc = (T) -1)
    {
        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;

        T matchedWc;
        if constexpr (MatchCutoff)
        {
            juce::ignoreUnused (matchedFc);
            matchedWc = wc;
        }
        else
        {
            matchedWc = matchedFc > (T) 0 ? (juce::MathConstants<NumericType>::twoPi * matchedFc) : wc;
        }

        if constexpr (mode == CoefficientCalculationMode::Standard)
        {
            const auto K = ConformalMaps::computeKValueAngular (matchedWc, fs);
            auto kSqTerm = (T) 1 / (wc * wc);
            auto kTerm = (T) 1 / (qVal * wc);
            ConformalMaps::Transform<T, 2>::bilinear (b, a, { kSqTerm, (T) 0, (T) 0 }, { kSqTerm, kTerm, (T) 1 }, K);
        }
        else if constexpr (mode == CoefficientCalculationMode::Decramped)
        {
            qVal = VicanekHelpers::clampQVicanek (qVal);
            if (SIMDUtils::any (fc < (T) 1000))
            {
                calcSecondOrderHPF<T, NumericType, MatchCutoff, CoefficientCalculationMode::Standard> (b, a, fc, qVal, fs, matchedFc);
                return;
            }

            const auto [p0, p1, p2, A0, A1, A2] = VicanekHelpers::computeVicanekPolesAngular (matchedWc, qVal, fs, a);

            CHOWDSP_USING_XSIMD_STD (sqrt);
            b[0] = sqrt (A0 * p0 + A1 * p1 + A2 * p2) * qVal / ((T) 4 * p1);
            b[1] = -(T) 2 * b[0];
            b[2] = b[0];
        }
    }

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, and sample rate.
     * The analog prototype transfer function is: \f$ H(s) = \frac{s/Q}{s^2 + s/Q + 1} \f$
     */
    template <typename T, typename NumericType, CoefficientCalculationMode mode = CoefficientCalculationMode::Standard>
    void calcSecondOrderBPF (T (&b)[3], T (&a)[3], T fc, T qVal, NumericType fs)
    {
        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;
        if constexpr (mode == CoefficientCalculationMode::Standard)
        {
            const auto K = ConformalMaps::computeKValueAngular (wc, fs);
            auto kSqTerm = (T) 1 / (wc * wc);
            auto kTerm = (T) 1 / (qVal * wc);
            ConformalMaps::Transform<T, 2>::bilinear (b, a, { (T) 0, kTerm, (T) 0 }, { kSqTerm, kTerm, (T) 1 }, K);
        }
        else if constexpr (mode == CoefficientCalculationMode::Decramped)
        {
            qVal = VicanekHelpers::clampQVicanek (qVal);
            if (SIMDUtils::any (fc < (T) 1000))
            {
                calcSecondOrderBPF<T, NumericType, CoefficientCalculationMode::Standard> (b, a, fc, qVal, fs);
                return;
            }

            const auto [p0, p1, p2, A0, A1, A2] = VicanekHelpers::computeVicanekPolesAngular (wc, qVal, fs, a);

            CHOWDSP_USING_XSIMD_STD (sqrt);
            const auto R1 = A0 * p0 + A1 * p1 + A2 * p2;
            const auto R2 = -A0 + A1 + (T) 4 * (p0 - p1) * A2;
            const auto B2 = (R1 - R2 * p1) / (4 * p1 * p1);
            const auto B1 = R2 + 4 * (p1 - p0) * B2;
            b[1] = -(T) 0.5 * sqrt (B1);
            b[0] = (T) 0.5 * (sqrt (B2 + (T) 0.25 * B1) - b[1]);
            b[2] = -b[0] - b[1];
        }
    }

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, and sample rate.
     * The analog prototype transfer function is: \f$ H(S) = \frac{s^2 + 1}{s^2 + s/Q + 1} \f$
     */
    template <typename T, typename NumericType, CoefficientCalculationMode mode = CoefficientCalculationMode::Standard>
    void calcNotchFilter (T (&b)[3], T (&a)[3], T fc, T qVal, NumericType fs)
    {
        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;
        if constexpr (mode == CoefficientCalculationMode::Standard)
        {
            const auto K = ConformalMaps::computeKValueAngular (wc, fs);
            auto kSqTerm = (T) 1 / (wc * wc);
            auto kTerm = (T) 1 / (qVal * wc);
            ConformalMaps::Transform<T, 2>::bilinear (b, a, { kSqTerm, (T) 0, (T) 1 }, { kSqTerm, kTerm, (T) 1 }, K);
        }
        else if constexpr (mode == CoefficientCalculationMode::Decramped)
        {
            qVal = VicanekHelpers::clampQVicanek (qVal);
            if (SIMDUtils::any (fc < (T) 1000))
            {
                calcNotchFilter<T, NumericType, CoefficientCalculationMode::Standard> (b, a, fc, qVal, fs);
                return;
            }

            CHOWDSP_USING_XSIMD_STD (sqrt);
            CHOWDSP_USING_XSIMD_STD (cos);
            using Power::ipow;

            const auto w0 = wc / fs;
            VicanekHelpers::computeVicanekDenominator (w0, qVal, a);

            const auto A0 = ipow<2> ((T) 1 + a[1] + a[2]);
            const auto b1_unscaled = (T) -2 * cos (w0);
            const auto scale = sqrt (A0) / ((T) 2 + b1_unscaled);

            b[0] = scale;
            b[1] = b1_unscaled * scale;
            b[2] = scale;
        }
    }

    /**
     * Forwards a parameters to a coefficient calculator meant to be used for linear gain units
     * instead of Decibel gain units.
     */
    template <typename T, typename NumericType, typename FilterType>
    void calcCoefsGainDB (FilterType& filter, T fc, T qVal, T gainDB, NumericType fs)
    {
        using SIMDUtils::decibelsToGain;
        filter.calcCoefs (fc, qVal, decibelsToGain (gainDB), fs);
    }

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, gain, and sample rate.
     * Note that the gain should be in units of linear gain, NOT Decibels.
     * The analog prototype transfer function is: \f$ H(s) = \frac{s^2 + G s/Q + 1}{s^2 + s/Q + 1} \f$
     */
    template <typename T, typename NumericType, CoefficientCalculationMode mode = CoefficientCalculationMode::Standard>
    void calcPeakingFilter (T (&b)[3], T (&a)[3], T fc, T qVal, T gain, NumericType fs)
    {
        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;
        if constexpr (mode == CoefficientCalculationMode::Standard)
        {
            const auto K = ConformalMaps::computeKValueAngular (wc, fs);
            const auto kSqTerm = (T) 1 / (wc * wc);
            const auto kTerm = (T) 1 / (qVal * wc);

            const auto kNum = SIMDUtils::select (gain > (T) 1, kTerm * gain, kTerm);
            const auto kDen = SIMDUtils::select (gain < (T) 1, kTerm / gain, kTerm);
            ConformalMaps::Transform<T, 2>::bilinear (b, a, { kSqTerm, kNum, (T) 1 }, { kSqTerm, kDen, (T) 1 }, K);
        }
        else if constexpr (mode == CoefficientCalculationMode::Decramped)
        {
            qVal = VicanekHelpers::clampQVicanek (qVal);
            if (SIMDUtils::any (fc < (T) 1000))
            {
                calcPeakingFilter<T, NumericType, CoefficientCalculationMode::Standard> (b, a, fc, qVal, gain, fs);
                return;
            }

            using SIMDUtils::select;

            const auto g = select (gain < (T) 1, (T) 1 / gain, gain);
            T a_copy[3] {};
            T b_copy[3] {};

            const auto [p0, p1, p2, A0, A1, A2] = VicanekHelpers::computeVicanekPolesAngular (wc, qVal, fs, a_copy);

            const auto G2 = Power::ipow<2> (g);
            const auto R1 = (A0 * p0 + A1 * p1 + A2 * p2) * G2;
            const auto R2 = (-A0 + A1 + (T) 4 * (p0 - p1) * A2) * G2;
            const auto B0 = A0;
            const auto B2 = (R1 - R2 * p1 - B0) / ((T) 4 * p1 * p1);
            const auto B1 = R2 + B0 + (T) 4 * (p1 - p0) * B2;

            VicanekHelpers::computeVicanekBiquadNumerator (B0, B1, B2, b_copy);

            // In order to match q-values with the Standard coefficient calculator,
            // we need to invert the coefficients, if the gain is less than 1.
            // The Vicanek paper asserts that the zeros will be inside the unit circle, so we
            // can safely invert the coefficients without having to worry about stability.
            a[0] = select (gain < (T) 1, b_copy[0], a_copy[0]);
            a[1] = select (gain < (T) 1, b_copy[1], a_copy[1]);
            a[2] = select (gain < (T) 1, b_copy[2], a_copy[2]);
            b[0] = select (gain < (T) 1, a_copy[0], b_copy[0]);
            b[1] = select (gain < (T) 1, a_copy[1], b_copy[1]);
            b[2] = select (gain < (T) 1, a_copy[2], b_copy[2]);

            const auto a_norm = (T) 1 / a[0];
            a[0] = (T) 1;
            a[1] *= a_norm;
            a[2] *= a_norm;
            b[0] *= a_norm;
            b[1] *= a_norm;
            b[2] *= a_norm;
        }
    }

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, gain, and sample rate.
     * Note that the gain should be in units of linear gain, NOT Decibels.
     * The analog prototype transfer function is: \f$ H(s) = A \frac{s^2 + \sqrt{A} s/Q + A}{A s^2 + \sqrt{A} s/Q + 1} \f$
     */
    template <typename T, typename NumericType, CoefficientCalculationMode mode = CoefficientCalculationMode::Standard>
    void calcLowShelf (T (&b)[3], T (&a)[3], T fc, T qVal, T gain, NumericType fs)
    {
        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;
        if constexpr (mode == CoefficientCalculationMode::Standard)
        {
            const auto K = ConformalMaps::computeKValueAngular (wc, fs);

            CHOWDSP_USING_XSIMD_STD (sqrt);
            const auto A = sqrt (gain);
            const auto Aroot = sqrt (A);

            auto kSqTerm = (T) 1 / (wc * wc);
            auto kTerm = Aroot / (qVal * wc);
            ConformalMaps::Transform<T, 2>::bilinear (b, a, { A * kSqTerm, A * kTerm, A * A }, { A * kSqTerm, kTerm, (T) 1 }, K);
        }
        else if constexpr (mode == CoefficientCalculationMode::Decramped)
        {
            qVal = VicanekHelpers::clampQVicanek (qVal);
            if (SIMDUtils::any (fc < (T) 1000))
            {
                calcLowShelf<T, NumericType, CoefficientCalculationMode::Standard> (b, a, fc, qVal, gain, fs);
                return;
            }

            // The Vicanek paper doesn't derive the shelving filters, so see the note in the high shelf function.
            // A similar approach is used here, except that the poles are different since the denominator of the
            // analog transfer function is different, plus the numerator and denominator of the G2 expression are
            // swapped, and B0 = A0 * gain^2

            CHOWDSP_USING_XSIMD_STD (sqrt);
            CHOWDSP_USING_XSIMD_STD (exp);
            using Power::ipow;
            using SIMDUtils::select;

            const auto g = select (gain < (T) 1, (T) 1 / gain, gain);
            T a_copy[3] {};
            T b_copy[3] {};

            const auto w0 = wc / fs;
            const auto Alpha = sqrt (g);
            const auto invAlpha = (T) 1 / Alpha;
            const auto Beta = sqrt (Alpha) / ((T) 2 * qVal);
            const auto BetaSq = ipow<2> (Beta);

            const auto expBw_A = exp (-Beta * w0 * invAlpha);
            const auto cos_cosh_arg = select (BetaSq <= Alpha, sqrt (Alpha - BetaSq), sqrt (BetaSq - Alpha)) * w0 * invAlpha;
            a_copy[0] = (T) 1;
            a_copy[1] = -(T) 2 * expBw_A * select (BetaSq <= Alpha, cos (cos_cosh_arg), cosh (cos_cosh_arg));
            a_copy[2] = ipow<2> (expBw_A);

            const auto f0 = juce::MathConstants<NumericType>::twoPi / w0;
            const auto w_N_Sq = ipow<2> ((T) 0.5 * f0);
            const auto C = ipow<2> (Beta * f0); // = (2*Beta)^2 w_N^2
            const auto G2 = g * (ipow<2> (Alpha - w_N_Sq) + C) / (ipow<2> (1 - Alpha * w_N_Sq) + C);

            const auto [p0, p1, p2] = VicanekHelpers::computeVicanekPhiVals (w0);
            const auto [A0, A1, A2] = VicanekHelpers::computeVicanekAVals (a_copy);

            const auto R1 = (A0 * p0 + A1 * p1 + A2 * p2) * g;
            const auto B0 = A0 * ipow<2> (g);
            const auto B1 = A1 * G2;
            const auto B2 = (R1 - B0 * p0 - B1 * p1) / p2;

            VicanekHelpers::computeVicanekBiquadNumerator (B0, B1, B2, b_copy);

            a[0] = select (gain < (T) 1, b_copy[0], a_copy[0]);
            a[1] = select (gain < (T) 1, b_copy[1], a_copy[1]);
            a[2] = select (gain < (T) 1, b_copy[2], a_copy[2]);
            b[0] = select (gain < (T) 1, a_copy[0], b_copy[0]);
            b[1] = select (gain < (T) 1, a_copy[1], b_copy[1]);
            b[2] = select (gain < (T) 1, a_copy[2], b_copy[2]);

            const auto a_norm = (T) 1 / a[0];
            a[0] = (T) 1;
            a[1] *= a_norm;
            a[2] *= a_norm;
            b[0] *= a_norm;
            b[1] *= a_norm;
            b[2] *= a_norm;
        }
    }

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, gain, and sample rate.
     * Note that the gain should be in units of linear gain, NOT Decibels.
     * The analog prototype transfer function is: \f$ H(s) = A \frac{As^2 + \sqrt{A} s/Q + 1}{s^2 + \sqrt{A} s/Q + A} \f$
     */
    template <typename T, typename NumericType, CoefficientCalculationMode mode = CoefficientCalculationMode::Standard>
    void calcHighShelf (T (&b)[3], T (&a)[3], T fc, T qVal, T gain, NumericType fs)
    {
        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;

        if constexpr (mode == CoefficientCalculationMode::Standard)
        {
            const auto K = ConformalMaps::computeKValueAngular (wc, fs);

            CHOWDSP_USING_XSIMD_STD (sqrt);
            const auto A = sqrt (gain);
            const auto Aroot = sqrt (A);

            auto kSqTerm = (T) 1 / (wc * wc);
            auto kTerm = Aroot / (qVal * wc);

            ConformalMaps::Transform<T, 2>::bilinear (b, a, { A * A * kSqTerm, A * kTerm, A }, { kSqTerm, kTerm, (T) A }, K);
        }
        else if constexpr (mode == CoefficientCalculationMode::Decramped)
        {
            qVal = VicanekHelpers::clampQVicanek (qVal);
            if (SIMDUtils::any (fc < (T) 1000))
            {
                calcHighShelf<T, NumericType, CoefficientCalculationMode::Standard> (b, a, fc, qVal, gain, fs);
                return;
            }

            // The Vicanek paper doesn't derive the shelving filters, so here's what we do:
            // - Compute the poles using impulse invariance
            //   - N.B. this is different from the poles used by the LPF and other filters, since the denominator of the analog transfer function is different
            // - We know the gain at DC = 1, so B0 = A0
            // - We can derive the squared gain at the Nyquist frequency (G2), so then B1 = A1 * G2
            // - Finally, we can match the gain at the cutoff frequency (like with the peaking filter), which gives us B2
            //
            // Things get a bit hairy numerically when we have a large boost near Nyquist, so we always
            // compute the "cut" case of the filter, and invert the coefficients if necessary.
            // The Vicanek paper asserts that the zeros will be inside the unit circle, so we can safely
            // invert the coefficients without having to worry about stability.

            CHOWDSP_USING_XSIMD_STD (sqrt);
            CHOWDSP_USING_XSIMD_STD (exp);
            using Power::ipow;
            using SIMDUtils::select;

            const auto g = select (gain > (T) 1, (T) 1 / gain, gain);
            T a_copy[3] {};
            T b_copy[3] {};

            const auto w0 = wc / fs;
            const auto Alpha = sqrt (g);
            const auto Beta = sqrt (Alpha) / ((T) 2 * qVal);
            const auto BetaSq = ipow<2> (Beta);

            const auto expBw = exp (-Beta * w0);
            const auto cos_cosh_arg = select (BetaSq <= Alpha, sqrt (Alpha - BetaSq), sqrt (BetaSq - Alpha)) * w0;
            a_copy[0] = (T) 1;
            a_copy[1] = -(T) 2 * expBw * SIMDUtils::select (BetaSq <= Alpha, cos (cos_cosh_arg), cosh (cos_cosh_arg));
            a_copy[2] = ipow<2> (expBw);

            const auto f0 = juce::MathConstants<NumericType>::twoPi / w0;
            const auto w_N_Sq = ipow<2> ((T) (NumericType) 0.49 * f0);
            const auto C = ipow<2> (Beta * f0); // = (2*Beta)^2 w_N^2
            const auto Gn2 = g * (ipow<2> (1 - Alpha * w_N_Sq) + C) / (ipow<2> (Alpha - w_N_Sq) + C);

            const auto [p0, p1, p2] = VicanekHelpers::computeVicanekPhiVals (w0);
            const auto [A0, A1, A2] = VicanekHelpers::computeVicanekAVals (a_copy);

            const auto R1 = (A0 * p0 + A1 * p1 + A2 * p2) * g;
            const auto B0 = A0;
            const auto B1 = A1 * Gn2;
            const auto B2 = (R1 - B0 * p0 - B1 * p1) / p2;

            VicanekHelpers::computeVicanekBiquadNumerator (B0, B1, B2, b_copy);

            a[0] = select (gain > (T) 1, b_copy[0], a_copy[0]);
            a[1] = select (gain > (T) 1, b_copy[1], a_copy[1]);
            a[2] = select (gain > (T) 1, b_copy[2], a_copy[2]);
            b[0] = select (gain > (T) 1, a_copy[0], b_copy[0]);
            b[1] = select (gain > (T) 1, a_copy[1], b_copy[1]);
            b[2] = select (gain > (T) 1, a_copy[2], b_copy[2]);

            const auto a_norm = (T) 1 / a[0];
            a[0] = (T) 1;
            a[1] *= a_norm;
            a[2] *= a_norm;
            b[0] *= a_norm;
            b[1] *= a_norm;
            b[2] *= a_norm;
        }
    }
} // namespace CoefficientCalculators
} // namespace chowdsp
