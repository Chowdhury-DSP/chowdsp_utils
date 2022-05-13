#pragma once

namespace chowdsp::SIMDUtils
{
// xsimd SIMD registers don't directly support a few of these methods.

/** Returns the maximum value from the SIMD register */
template <typename T>
inline T hMaxSIMD (const xsimd::batch<T>& x)
{
    constexpr auto vecSize = xsimd::batch<T>::size;
    T v alignas (xsimd::default_arch::alignment())[vecSize];
    xsimd::store_aligned (v, x);

    if constexpr (vecSize == 2)
        return juce::jmax (v[0], v[1]);
    else if constexpr (vecSize == 4)
        return juce::jmax (v[0], v[1], v[2], v[3]);
    else
        return juce::jmax (juce::jmax (v[0], v[1], v[2], v[3]), juce::jmax (v[4], v[5], v[6], v[7]));
}

/** Returns the minimum value from the SIMD register */
template <typename T>
inline T hMinSIMD (const xsimd::batch<T>& x)
{
    constexpr auto vecSize = xsimd::batch<T>::size;
    T v alignas (xsimd::default_arch::alignment())[vecSize];
    xsimd::store_aligned (v, x);

    if constexpr (vecSize == 2)
        return juce::jmin (v[0], v[1]);
    else if constexpr (vecSize == 4)
        return juce::jmin (v[0], v[1], v[2], v[3]);
    else
        return juce::jmin (juce::jmin (v[0], v[1], v[2], v[3]), juce::jmin (v[4], v[5], v[6], v[7]));
}

/** Returns the maximum absolute value from the SIMD register */
template <typename T>
inline T hAbsMaxSIMD (const xsimd::batch<T>& x)
{
    return hMaxSIMD (xsimd::abs (x));
}
} // namespace chowdsp::SIMDUtils
