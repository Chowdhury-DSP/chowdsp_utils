#pragma once

namespace chowdsp::SIMDUtils
{
/** Scalar specialization of juce::Decibels::gainToDecibels */
template <typename T>
inline T gainToDecibels (T gain, T minusInfinityDB = (T) -100.0)
{
    return juce::Decibels::gainToDecibels (gain, minusInfinityDB);
}

/** Scalar specialization of juce::Decibels::decibelsToGain */
template <typename T>
inline T decibelsToGain (T decibels, T minusInfinityDB = (T) -100.0)
{
    return juce::Decibels::decibelsToGain (decibels, minusInfinityDB);
}

#if ! CHOWDSP_NO_XSIMD
/** SIMD specialization of juce::Decibels::gainToDecibels */
template <typename T>
inline xsimd::batch<T> gainToDecibels (const xsimd::batch<T>& gain, T minusInfinityDB = (T) -100.0)
{
    using v_type = xsimd::batch<T>;
    return xsimd::select (gain > (T) 0, xsimd::max (xsimd::log10 (gain) * (T) 20, (v_type) minusInfinityDB), (v_type) minusInfinityDB);
}

/** SIMD specialization of juce::Decibels::decibelsToGain */
template <typename T>
inline xsimd::batch<T> decibelsToGain (const xsimd::batch<T>& decibels, T minusInfinityDB = (T) -100.0)
{
    return xsimd::select (decibels > minusInfinityDB, xsimd::pow ((xsimd::batch<T>) 10, decibels * (T) 0.05), {});
}
#endif
} // namespace chowdsp::SIMDUtils
