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
        using namespace Bilinear;
        using namespace SIMDUtils;

        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;
        const auto K = computeKValue (fc, fs);
        BilinearTransform<T, 2>::call (b, a, { (T) 0, (T) 1 }, { (T) 1 / wc, (T) 1 }, K);
    }

    /**
     * Calculates the filter coefficients for a given cutoff frequency and sample rate.
     * The analog prototype transfer function is: \f$ H(s) = \frac{s}{s + 1} \f$
     */
    template <typename T, typename NumericType>
    void calcFirstOrderHPF (T (&b)[2], T (&a)[2], T fc, NumericType fs)
    {
        using namespace Bilinear;
        using namespace SIMDUtils;

        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;
        const auto K = computeKValue (fc, fs);
        BilinearTransform<T, 2>::call (b, a, { (T) 1 / wc, (T) 0 }, { (T) 1 / wc, (T) 1 }, K);
    }

    /** Calculates the coefficients for the filter.
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
        if (lowGain == highGain)
        {
            b[0] = lowGain;
            b[1] = (T) 0;
            a[0] = (T) 1;
            a[1] = (T) 0;
            return;
        }

        using namespace SIMDUtils;

        T rho {}, K;
        if constexpr (std::is_floating_point_v<T>)
        {
            rho = std::sqrt (highGain / lowGain);
            K = (T) 1 / std::tan (juce::MathConstants<NumericType>::pi * fc / fs);
        }
        else if constexpr (SampleTypeHelpers::IsSIMDRegister<T>)
        {
            rho = sqrtSIMD (highGain / lowGain);
            K = (T) 1 / tanSIMD (juce::MathConstants<NumericType>::pi * fc / fs);
        }

        Bilinear::BilinearTransform<T, 2>::call (b, a, { highGain / rho, lowGain }, { 1.0f / rho, 1.0f }, K);
    }

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, and sample rate.
     * The analog prototype transfer function is: \f$ H(s) = \frac{1}{s^2 + s/Q + 1} \f$
     */
    template <typename T, typename NumericType, bool MatchCutoff = true>
    void calcSecondOrderLPF (T (&b)[3], T (&a)[3], T fc, T qVal, NumericType fs, T matchedFc = (T) -1)
    {
        using namespace Bilinear;
        using namespace SIMDUtils;

        if constexpr (MatchCutoff)
            matchedFc = fc;
        else
            matchedFc = matchedFc > (T) 0 ? matchedFc : fc;

        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;
        const auto K = computeKValue (matchedFc, fs);

        auto kSqTerm = (T) 1 / (wc * wc);
        auto kTerm = (T) 1 / (qVal * wc);

        BilinearTransform<T, 3>::call (b, a, { (T) 0, (T) 0, (T) 1 }, { kSqTerm, kTerm, (T) 1 }, K);
    }

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, and sample rate.
     * The analog prototype transfer function is: \f$ H(s) = \frac{s^2}{s^2 + s/Q + 1} \f$
     */
    template <typename T, typename NumericType, bool MatchCutoff = true>
    void calcSecondOrderHPF (T (&b)[3], T (&a)[3], T fc, T qVal, NumericType fs, T matchedFc = (T) -1)
    {
        using namespace Bilinear;
        using namespace SIMDUtils;

        if constexpr (MatchCutoff)
            matchedFc = fc;
        else
            matchedFc = matchedFc > (T) 0 ? matchedFc : fc;

        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;
        const auto K = computeKValue (matchedFc, fs);

        auto kSqTerm = (T) 1 / (wc * wc);
        auto kTerm = (T) 1 / (qVal * wc);

        BilinearTransform<T, 3>::call (b, a, { kSqTerm, (T) 0, (T) 0 }, { kSqTerm, kTerm, (T) 1 }, K);
    }

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, and sample rate.
     * The analog prototype transfer function is: \f$ H(s) = \frac{s/Q}{s^2 + s/Q + 1} \f$
     */
    template <typename T, typename NumericType>
    void calcSecondOrderBPF (T (&b)[3], T (&a)[3], T fc, T qVal, NumericType fs)
    {
        using namespace Bilinear;
        using namespace SIMDUtils;

        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;
        const auto K = computeKValue (fc, fs);

        auto kSqTerm = (T) 1 / (wc * wc);
        auto kTerm = (T) 1 / (qVal * wc);

        BilinearTransform<T, 3>::call (b, a, { (T) 0, kTerm, (T) 0 }, { kSqTerm, kTerm, (T) 1 }, K);
    }

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, and sample rate.
     * The analog prototype transfer function is: \f$ H(S) = \frac{s^2 + 1}{s^2 + s/Q + 1} \f$
     */
    template <typename T, typename NumericType>
    void calcNotchFilter (T (&b)[3], T (&a)[3], T fc, T qVal, NumericType fs)
    {
        using namespace Bilinear;
        using namespace SIMDUtils;

        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;
        const auto K = computeKValue (fc, fs);

        auto kSqTerm = (T) 1 / (wc * wc);
        auto kTerm = (T) 1 / (qVal * wc);

        BilinearTransform<T, 3>::call (b, a, { kSqTerm, (T) 0, (T) 1 }, { kSqTerm, kTerm, (T) 1 }, K);
    }

    /**
     * Forwards a parameters to a coefficient calculator meant to be used for linear gain units
     * instead of Decibel gain units.
     */
    template <typename T, typename NumericType, typename FilterType>
    void calcCoefsGainDB (FilterType& filter, T fc, T qVal, T gainDB, NumericType fs)
    {
        if constexpr (std::is_floating_point_v<T>)
            filter.calcCoefs (fc, qVal, juce::Decibels::decibelsToGain (gainDB), fs);
        else if constexpr (SampleTypeHelpers::IsSIMDRegister<T>)
            filter.calcCoefs (fc, qVal, SIMDUtils::decibelsToGain (gainDB), fs);
    }

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, gain, and sample rate.
     * Note that the gain should be in units of linear gain, NOT Decibels.
     * The analog prototype transfer function is: \f$ H(s) = \frac{s^2 + G s/Q + 1}{s^2 + s/Q + 1} \f$
     */
    template <typename T, typename NumericType>
    void calcPeakingFilter (T (&b)[3], T (&a)[3], T fc, T qVal, T gain, NumericType fs)
    {
        using namespace Bilinear;
        using namespace SIMDUtils;

        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;
        const auto K = computeKValue (fc, fs);

        auto kSqTerm = (T) 1 / (wc * wc);
        auto kTerm = (T) 1 / (qVal * wc);

        T kNum {}, kDen {};
        if constexpr (std::is_floating_point_v<T>)
        {
            kNum = gain > (T) 1 ? kTerm * gain : kTerm;
            kDen = gain < (T) 1 ? kTerm / gain : kTerm;
        }
        else if constexpr (SampleTypeHelpers::IsSIMDRegister<T>)
        {
            kNum = select (T::greaterThan (gain, (T) 1), kTerm * gain, kTerm);
            kDen = select (T::lessThan (gain, (T) 1), kTerm / gain, kTerm);
        }

        BilinearTransform<T, 3>::call (b, a, { kSqTerm, kNum, (T) 1 }, { kSqTerm, kDen, (T) 1 }, K);
    }

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, gain, and sample rate.
     * Note that the gain should be in units of linear gain, NOT Decibels.
     * The analog prototype transfer function is: \f$ H(s) = A \frac{s^2 + \sqrt{A} s/Q + A}{A s^2 + \sqrt{A} s/Q + 1} \f$
     */
    template <typename T, typename NumericType>
    void calcLowShelf (T (&b)[3], T (&a)[3], T fc, T qVal, T gain, NumericType fs)
    {
        using namespace Bilinear;
        using namespace SIMDUtils;

        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;
        const auto K = computeKValue (fc, fs);

        T A {}, Aroot {};
        if constexpr (std::is_floating_point_v<T>)
        {
            A = std::sqrt (gain);
            Aroot = std::sqrt (A);
        }
        else if constexpr (SampleTypeHelpers::IsSIMDRegister<T>)
        {
            A = sqrtSIMD (gain);
            Aroot = sqrtSIMD (A);
        }

        auto kSqTerm = (T) 1 / (wc * wc);
        auto kTerm = Aroot / (qVal * wc);

        BilinearTransform<T, 3>::call (b, a, { A * kSqTerm, A * kTerm, A * A }, { A * kSqTerm, kTerm, (T) 1 }, K);
    }

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, gain, and sample rate.
     * Note that the gain should be in units of linear gain, NOT Decibels.
     * The analog prototype transfer function is: \f$ H(s) = A \frac{As^2 + \sqrt{A} s/Q + 1}{s^2 + \sqrt{A} s/Q + A} \f$
     */
    template <typename T, typename NumericType>
    void calcHighShelf (T (&b)[3], T (&a)[3], T fc, T qVal, T gain, NumericType fs)
    {
        using namespace Bilinear;
        using namespace SIMDUtils;

        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;
        const auto K = computeKValue (fc, fs);

        T A {}, Aroot {};
        if constexpr (std::is_floating_point_v<T>)
        {
            A = std::sqrt (gain);
            Aroot = std::sqrt (A);
        }
        else if constexpr (SampleTypeHelpers::IsSIMDRegister<T>)
        {
            A = sqrtSIMD (gain);
            Aroot = sqrtSIMD (A);
        }

        auto kSqTerm = (T) 1 / (wc * wc);
        auto kTerm = Aroot / (qVal * wc);

        BilinearTransform<T, 3>::call (b, a, { A * A * kSqTerm, A * kTerm, A }, { kSqTerm, kTerm, (T) A }, K);
    }
} // namespace CoefficientCalculators
} // namespace chowdsp
