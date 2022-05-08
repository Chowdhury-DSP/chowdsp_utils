#pragma once

namespace chowdsp::SIMDUtils
{
/** SIMD specialization of juce::Decibels::gainToDecibels */
template <typename T>
inline juce::dsp::SIMDRegister<T> gainToDecibels (const juce::dsp::SIMDRegister<T>& gain, T minusInfinityDB = (T) -100.0)
{
    using Vec = juce::dsp::SIMDRegister<T>;
    auto gZero = Vec::greaterThan (gain, (T) 0.0);
    return select (gZero, Vec::max (log10SIMD (gain) * (T) 20.0, minusInfinityDB), (Vec) minusInfinityDB);
}

/** SIMD specialization of juce::Decibels::decibelsToGain */
template <typename T>
inline juce::dsp::SIMDRegister<T> decibelsToGain (const juce::dsp::SIMDRegister<T>& decibels, T minusInfinityDB = (T) -100.0)
{
    using Vec = juce::dsp::SIMDRegister<T>;
    auto gZero = Vec::greaterThan (decibels, minusInfinityDB);
    return select (gZero, powSIMD ((Vec) (T) 10.0, decibels * (T) 0.05), Vec());
}
} // namespace chowdsp::SIMDUtils
