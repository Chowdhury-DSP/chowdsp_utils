#pragma once

namespace chowdsp
{
/** A first order shelving filter, with a set gain at DC,
 * a set gain at high frequencies, and a transition frequency.
 */
template <typename T = float>
class ShelfFilter : public IIRFilter<1, T>
{
    using NumericType = typename SampleTypeHelpers::ElementType<T>::Type;

public:
    ShelfFilter() = default;

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
        if constexpr (std::is_floating_point<T>::value)
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
