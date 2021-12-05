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
    FirstOrderLPF() = default;

    /**
     * Calculates the filter coefficients for a given cutoff frequency and sample rate.
     * The analog prototype transfer function is:
     *            1
     *  H(s) = ---------
     *         s  +  1
     */
    void calcCoefs (T fc, T fs)
    {
        using namespace Bilinear;

        const auto wc = juce::MathConstants<T>::twoPi * fc;
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
    FirstOrderHPF() = default;

    /**
     * Calculates the filter coefficients for a given cutoff frequency and sample rate.
     * The analog prototype transfer function is:
     *            s
     *  H(s) = ---------
     *         s  +  1
     */
    void calcCoefs (T fc, T fs)
    {
        using namespace Bilinear;

        const auto wc = juce::MathConstants<T>::twoPi * fc;
        const auto K = computeKValue (fc, fs);
        BilinearTransform<T, 2>::call (this->b, this->a, { (T) 1 / wc, (T) 0 }, { (T) 1 / wc, (T) 1 }, K);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FirstOrderHPF)
};

} // namespace chowdsp
