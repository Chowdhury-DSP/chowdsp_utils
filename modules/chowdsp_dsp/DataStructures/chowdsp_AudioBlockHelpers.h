#pragma once

/** Helper methods for juce::dsp::AudioBlock */
namespace chowdsp::AudioBlockHelpers
{
/** Copy the information from one block to another (float/double) */
template <typename T>
inline typename std::enable_if<std::is_floating_point<T>::value, void>::type
    copyBlocks (juce::dsp::AudioBlock<T>& dest, const juce::dsp::AudioBlock<const T>& src)
{
    dest.copyFrom (src);
}

/** Copy the information from one block to another (SIMD types) */
template <typename T>
inline typename std::enable_if<SampleTypeHelpers::IsSIMDRegister<T>, void>::type
    copyBlocks (juce::dsp::AudioBlock<T>& dest, const juce::dsp::AudioBlock<const T>& src)
{
    for (size_t ch = 0; ch < dest.getNumChannels(); ++ch)
    {
        auto* srcPtr = src.getChannelPointer (ch);
        auto* destPtr = dest.getChannelPointer (ch);
        for (size_t i = 0; i < dest.getNumSamples(); ++i)
            destPtr[i] = srcPtr[i];
    }
}

} // namespace chowdsp::AudioBlockHelpers
