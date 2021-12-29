#pragma once

namespace chowdsp::SIMDUtils
{
/** SIMD specialization of juce::Decibels::gainToDecibels */
template <typename T>
inline juce::dsp::SIMDRegister<T> gainToDecibels (const juce::dsp::SIMDRegister<T>& gain, T minusInfinityDB = (T) -100.0)
{
    using vec = juce::dsp::SIMDRegister<T>;
    auto gZero = vec::greaterThan (gain, (T) 0.0);
    return (vec::max (log10SIMD (gain) * (T) 20.0, minusInfinityDB) & gZero) + ((vec) minusInfinityDB & ~gZero);
}

/** SIMD specialization of juce::Decibels::decibelsToGain */
template <typename T>
inline juce::dsp::SIMDRegister<T> decibelsToGain (const juce::dsp::SIMDRegister<T>& decibels, T minusInfinityDB = (T) -100.0)
{
    using vec = juce::dsp::SIMDRegister<T>;
    auto gZero = vec::greaterThan (decibels, minusInfinityDB);
    return (powSIMD ((vec) (T) 10.0, decibels * (T) 0.05) & gZero) + ((vec) (T) 0.0 & ~gZero);
}
} // namespace chowdsp::SIMDUtils
