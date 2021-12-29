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

/** Ternary select operation */
template <typename T>
inline juce::dsp::SIMDRegister<T> select (MaskType<T> b, juce::dsp::SIMDRegister<T> t, juce::dsp::SIMDRegister<T> f)
{
    return (t & b) + (f & ~b);
}

} // namespace chowdsp::SIMDUtils
