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
    using NumericType = typename SampleTypeHelpers::ElementType<T>::Type;

public:
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
    using NumericType = typename SampleTypeHelpers::ElementType<T>::Type;

public:
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

} // namespace chowdsp
