#pragma once

namespace chowdsp
{
/**
 * First-order lowpass filter (-6 dB / octave).
 * This filter can be used to model an ideal RC lowpwass filter.
 */
template <typename T>
class FirstOrderLPF final : public chowdsp::IIRFilter<1, T>
{
public:
    using NumericType = SampleTypeHelpers::ProcessorNumericType<FirstOrderLPF>;

    FirstOrderLPF() = default;
    FirstOrderLPF (FirstOrderLPF&&) noexcept = default;
    FirstOrderLPF& operator= (FirstOrderLPF&&) noexcept = default;

    /**
     * Calculates the filter coefficients for a given cutoff frequency and sample rate.
     * The analog prototype transfer function is: \f$ H(s) = \frac{1}{s + 1} \f$
     */
    void calcCoefs (T fc, NumericType fs)
    {
        using namespace Bilinear;
        using namespace SIMDUtils;

        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;
        const auto K = computeKValue (fc, fs);
        BilinearTransform<T, 2>::call (this->b, this->a, { (T) 0, (T) 1 }, { (T) 1 / wc, (T) 1 }, K);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FirstOrderLPF)
};

/**
 * First-order highpass filter (-6 dB / octave).
 * This filter can be used to model an ideal RC highpass filter.
 */
template <typename T>
class FirstOrderHPF final : public chowdsp::IIRFilter<1, T>
{
public:
    using NumericType = SampleTypeHelpers::ProcessorNumericType<FirstOrderHPF>;

    FirstOrderHPF() = default;
    FirstOrderHPF (FirstOrderHPF&&) noexcept = default;
    FirstOrderHPF& operator= (FirstOrderHPF&&) noexcept = default;

    /**
     * Calculates the filter coefficients for a given cutoff frequency and sample rate.
     * The analog prototype transfer function is: \f$ H(s) = \frac{s}{s + 1} \f$
     */
    void calcCoefs (T fc, NumericType fs)
    {
        using namespace Bilinear;
        using namespace SIMDUtils;

        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;
        const auto K = computeKValue (fc, fs);
        BilinearTransform<T, 2>::call (this->b, this->a, { (T) 1 / wc, (T) 0 }, { (T) 1 / wc, (T) 1 }, K);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FirstOrderHPF)
};

/**
 * A first order shelving filter, with a set gain at DC,
 * a set gain at high frequencies, and a transition frequency.
 */
template <typename T = float>
class ShelfFilter final : public IIRFilter<1, T>
{
public:
    using NumericType = SampleTypeHelpers::ProcessorNumericType<ShelfFilter>;

    ShelfFilter() = default;
    ShelfFilter (ShelfFilter&&) noexcept = default;
    ShelfFilter& operator= (ShelfFilter&&) noexcept = default;

    /** Calculates the coefficients for the filter.
     * @param lowGain: the gain of the filter at low frequencies
     * @param highGain: the gain of the filter at high frequencies
     * @param fc: the transition frequency of the filter
     * @param fs: the sample rate for the filter
     *
     * For information on the filter coefficient derivation,
     * see Abel and Berners dsp4dae, pg. 249
     */
    void calcCoefs (T lowGain, T highGain, T fc, NumericType fs)
    {
        // reduce to simple gain element
        if (lowGain == highGain)
        {
            this->b[0] = lowGain;
            this->b[1] = (T) 0;
            this->a[0] = (T) 1;
            this->a[1] = (T) 0;
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

        Bilinear::BilinearTransform<T, 2>::call (this->b, this->a, { highGain / rho, lowGain }, { 1.0f / rho, 1.0f }, K);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShelfFilter)
};
} // namespace chowdsp
