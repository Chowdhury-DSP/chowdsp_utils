#pragma once

namespace chowdsp::SIMDUtils
{
template <typename T>
using MaskType = typename juce::dsp::SIMDRegister<T>::vMaskType;

/** Logical ANDNOT operation */
template <typename B>
inline B andnot (B a, B b)
{
    return a & (~b);
}

#if CHOWDSP_USE_XSIMD // XSIMD implementations
template <typename T>
using x_type = xsimd::batch<T, juce::dsp::SIMDRegister<T>::size()>;

template <typename T>
using b_type = xsimd::batch_bool<T, juce::dsp::SIMDRegister<T>::size()>;

/** Wrapper of xsimd::select, for ternary logic */
template <typename T>
inline juce::dsp::SIMDRegister<T> select (MaskType<T> b, juce::dsp::SIMDRegister<T> t, juce::dsp::SIMDRegister<T> f)
{
    return (juce::dsp::SIMDRegister<T>) xsimd::select ((b_type<T>) b.value, (x_type<T>) t.value, (x_type<T>) f.value);
}

#if (! CHOWDSP_USE_CUSTOM_JUCE_DSP) && (defined(_M_ARM64) || defined(__arm64__) || defined(__aarch64__))
/** Ternary select operation */
template <>
inline juce::dsp::SIMDRegister<double>::vMaskType isnanSIMD (juce::dsp::SIMDRegister<double> x)
{
    return (t & b) + (f & ~b);
}
#endif
#else // ! CHOWDSP_USE_XSIMD
/** Ternary select operation */
template <typename T>
inline juce::dsp::SIMDRegister<T> select (MaskType<T> b, juce::dsp::SIMDRegister<T> t, juce::dsp::SIMDRegister<T> f)
{
    return (t & b) + (f & ~b);
}
#endif // CHOWDSP_USE_XSIMD

} // namespace chowdsp::SIMDUtils
