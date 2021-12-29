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

/** Wrapper of xsimd::select, for ternary logic */
template <typename T>
inline juce::dsp::SIMDRegister<T> select (MaskType<T> b, juce::dsp::SIMDRegister<T> t, juce::dsp::SIMDRegister<T> f)
{
    return (juce::dsp::SIMDRegister<T>) xsimd::select (b.value, (x_type<T>) t.value, (x_type<T>) f.value);
}
#else
/** Ternary select operation */
template <typename T>
inline juce::dsp::SIMDRegister<T> select (MaskType<T> b, juce::dsp::SIMDRegister<T> t, juce::dsp::SIMDRegister<T> f)
{
    return (t & b) + (f & ~b);
}
#endif // CHOWDSP_USE_XSIMD

} // namespace chowdsp::SIMDUtils
