#pragma once

namespace chowdsp
{
template <typename T>
class FirstOrderLPF final : public chowdsp::IIRFilter<1, T>
{
public:
    FirstOrderLPF() = default;

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

template <typename T>
class FirstOrderHPF final : public chowdsp::IIRFilter<1, T>
{
public:
    FirstOrderHPF() = default;

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
