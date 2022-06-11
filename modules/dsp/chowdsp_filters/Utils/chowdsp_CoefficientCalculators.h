#pragma once

namespace chowdsp
{
/** Methods for computing coefficients for canonical filter forms */
namespace CoefficientCalculators
{
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
        if (SIMDUtils::all (lowGain == highGain))
        {
            b[0] = lowGain;
            b[1] = (T) 0;
            a[0] = (T) 1;
            a[1] = (T) 0;
            return;
        }

        CHOWDSP_USING_XSIMD_STD (sqrt)
        CHOWDSP_USING_XSIMD_STD (tan)

        const auto rho_recip = (T) 1 / sqrt (highGain / lowGain);
        const auto K = (T) 1 / tan (juce::MathConstants<NumericType>::pi * fc / fs);

        ConformalMaps::Transform<T, 1>::bilinear (b, a, { highGain * rho_recip, lowGain }, { rho_recip, (T) 1 }, K);
    }

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, and sample rate.
     * The analog prototype transfer function is: \f$ H(s) = \frac{1}{s^2 + s/Q + 1} \f$
     */
    template <typename T, typename NumericType, bool MatchCutoff = true>
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

        const auto K = ConformalMaps::computeKValueAngular (matchedWc, fs);

        auto kSqTerm = (T) 1 / (wc * wc);
        auto kTerm = (T) 1 / (qVal * wc);

        ConformalMaps::Transform<T, 2>::bilinear (b, a, { (T) 0, (T) 0, (T) 1 }, { kSqTerm, kTerm, (T) 1 }, K);
    }

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, and sample rate.
     * The analog prototype transfer function is: \f$ H(s) = \frac{s^2}{s^2 + s/Q + 1} \f$
     */
    template <typename T, typename NumericType, bool MatchCutoff = true>
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

        const auto K = ConformalMaps::computeKValueAngular (matchedWc, fs);

        auto kSqTerm = (T) 1 / (wc * wc);
        auto kTerm = (T) 1 / (qVal * wc);

        ConformalMaps::Transform<T, 2>::bilinear (b, a, { kSqTerm, (T) 0, (T) 0 }, { kSqTerm, kTerm, (T) 1 }, K);
    }

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, and sample rate.
     * The analog prototype transfer function is: \f$ H(s) = \frac{s/Q}{s^2 + s/Q + 1} \f$
     */
    template <typename T, typename NumericType>
    void calcSecondOrderBPF (T (&b)[3], T (&a)[3], T fc, T qVal, NumericType fs)
    {
        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;
        const auto K = ConformalMaps::computeKValueAngular (wc, fs);

        auto kSqTerm = (T) 1 / (wc * wc);
        auto kTerm = (T) 1 / (qVal * wc);

        ConformalMaps::Transform<T, 2>::bilinear (b, a, { (T) 0, kTerm, (T) 0 }, { kSqTerm, kTerm, (T) 1 }, K);
    }

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, and sample rate.
     * The analog prototype transfer function is: \f$ H(S) = \frac{s^2 + 1}{s^2 + s/Q + 1} \f$
     */
    template <typename T, typename NumericType>
    void calcNotchFilter (T (&b)[3], T (&a)[3], T fc, T qVal, NumericType fs)
    {
        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;
        const auto K = ConformalMaps::computeKValueAngular (wc, fs);

        auto kSqTerm = (T) 1 / (wc * wc);
        auto kTerm = (T) 1 / (qVal * wc);

        ConformalMaps::Transform<T, 2>::bilinear (b, a, { kSqTerm, (T) 0, (T) 1 }, { kSqTerm, kTerm, (T) 1 }, K);
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
    template <typename T, typename NumericType>
    void calcPeakingFilter (T (&b)[3], T (&a)[3], T fc, T qVal, T gain, NumericType fs)
    {
        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;
        const auto K = ConformalMaps::computeKValueAngular (wc, fs);

        const auto kSqTerm = (T) 1 / (wc * wc);
        const auto kTerm = (T) 1 / (qVal * wc);

        const auto kNum = SIMDUtils::select (gain > (T) 1, kTerm * gain, kTerm);
        const auto kDen = SIMDUtils::select (gain < (T) 1, kTerm / gain, kTerm);

        ConformalMaps::Transform<T, 2>::bilinear (b, a, { kSqTerm, kNum, (T) 1 }, { kSqTerm, kDen, (T) 1 }, K);
    }

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, gain, and sample rate.
     * Note that the gain should be in units of linear gain, NOT Decibels.
     * The analog prototype transfer function is: \f$ H(s) = A \frac{s^2 + \sqrt{A} s/Q + A}{A s^2 + \sqrt{A} s/Q + 1} \f$
     */
    template <typename T, typename NumericType>
    void calcLowShelf (T (&b)[3], T (&a)[3], T fc, T qVal, T gain, NumericType fs)
    {
        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;
        const auto K = ConformalMaps::computeKValueAngular (wc, fs);

        CHOWDSP_USING_XSIMD_STD (sqrt);
        const auto A = sqrt (gain);
        const auto Aroot = sqrt (A);

        auto kSqTerm = (T) 1 / (wc * wc);
        auto kTerm = Aroot / (qVal * wc);

        ConformalMaps::Transform<T, 2>::bilinear (b, a, { A * kSqTerm, A * kTerm, A * A }, { A * kSqTerm, kTerm, (T) 1 }, K);
    }

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, gain, and sample rate.
     * Note that the gain should be in units of linear gain, NOT Decibels.
     * The analog prototype transfer function is: \f$ H(s) = A \frac{As^2 + \sqrt{A} s/Q + 1}{s^2 + \sqrt{A} s/Q + A} \f$
     */
    template <typename T, typename NumericType>
    void calcHighShelf (T (&b)[3], T (&a)[3], T fc, T qVal, T gain, NumericType fs)
    {
        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;
        const auto K = ConformalMaps::computeKValueAngular (wc, fs);

        CHOWDSP_USING_XSIMD_STD (sqrt);
        const auto A = sqrt (gain);
        const auto Aroot = sqrt (A);

        auto kSqTerm = (T) 1 / (wc * wc);
        auto kTerm = Aroot / (qVal * wc);

        ConformalMaps::Transform<T, 2>::bilinear (b, a, { A * A * kSqTerm, A * kTerm, A }, { kSqTerm, kTerm, (T) A }, K);
    }
} // namespace CoefficientCalculators
} // namespace chowdsp
