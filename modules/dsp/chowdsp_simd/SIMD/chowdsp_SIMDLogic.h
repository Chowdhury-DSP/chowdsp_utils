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
inline juce::dsp::SIMDRegister<T> select (SampleTypeHelpers::vMaskTypeSIMD<T> b, juce::dsp::SIMDRegister<T> t, juce::dsp::SIMDRegister<T> f)
{
    return (t & b) + (f & ~b);
}

/** Returns true if any of the registers in the mask are true */
template <typename VecBoolType>
inline bool any (VecBoolType b)
{
    return b.sum() != 0;
}

/** Returns true if all of the registers in the mask are true */
template <typename VecBoolType>
inline bool all (VecBoolType b)
{
    // So this is a bit of a trick. When you do a boolean operation
    // with JUCE's SIMD registers, you get back a SIMDRegister with
    // unsigned int values of the same width (i.e. float -> uin32_t),
    // where the "true" values are the max_int for that type (maskVal).
    // When we add max_int + max_int of unsigned data types, with overflow,
    // we expect to get max_int - 1. With that in mind, we can figure out
    // the expected value of summing all the registers of the VecBoolType
    // (maskAllSum), and just compare against that.
    //
    // WARNING: if JUCE changes their strategy in the future, or if some
    // processor architectures handle overflow differently, this may no longer work.

    constexpr auto maskVal = std::numeric_limits<typename VecBoolType::ElementType>::max();
    constexpr auto maskAllSum = maskVal - (VecBoolType::size() - 1);
    return b.sum() == maskAllSum;
}
} // namespace chowdsp::SIMDUtils
