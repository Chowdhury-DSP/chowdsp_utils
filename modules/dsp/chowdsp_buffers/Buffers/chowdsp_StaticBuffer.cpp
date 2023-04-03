#include "chowdsp_StaticBuffer.h"

namespace chowdsp
{
template <typename SampleType, int maxNumChannels, int maxNumSamples>
StaticBuffer<SampleType, maxNumChannels, maxNumSamples>::StaticBuffer (int numChannels, int numSamples)
{
    setMaxSize (numChannels, numSamples);
}

template <typename SampleType, int maxNumChannels, int maxNumSamples>
void StaticBuffer<SampleType, maxNumChannels, maxNumSamples>::setMaxSize (int numChannels, int numSamples)
{
    jassert (juce::isPositiveAndNotGreaterThan (numChannels, maxNumChannels));
    jassert (juce::isPositiveAndNotGreaterThan (numSamples, maxNumSamples));

    hasBeenCleared = true;
    currentNumChannels = 0;
    currentNumSamples = 0;

    std::fill (channelPointers.begin(), channelPointers.end(), nullptr);
    for (int ch = 0; ch < numChannels; ++ch)
        channelPointers[(size_t) ch] = channelData[(size_t) ch].data();

    setCurrentSize (numChannels, numSamples);
}

template <typename SampleType, int maxNumChannels, int maxNumSamples>
void StaticBuffer<SampleType, maxNumChannels, maxNumSamples>::setCurrentSize (int numChannels, int numSamples) noexcept
{
    const auto increasingNumChannels = numChannels > currentNumChannels;
    const auto increasingNumSamples = numSamples > currentNumSamples;

    if (increasingNumSamples)
        buffer_detail::clear (channelPointers.data(), 0, currentNumChannels, currentNumSamples, numSamples);

    if (increasingNumChannels)
        buffer_detail::clear (channelPointers.data(), currentNumChannels, numChannels, 0, numSamples);

    currentNumChannels = numChannels;
    currentNumSamples = numSamples;
}

template <typename SampleType, int maxNumChannels, int maxNumSamples>
SampleType* StaticBuffer<SampleType, maxNumChannels, maxNumSamples>::getWritePointer (int channel) noexcept
{
    hasBeenCleared = false;
    return channelPointers[(size_t) channel];
}

template <typename SampleType, int maxNumChannels, int maxNumSamples>
const SampleType* StaticBuffer<SampleType, maxNumChannels, maxNumSamples>::getReadPointer (int channel) const noexcept
{
    return channelPointers[(size_t) channel];
}

template <typename SampleType, int maxNumChannels, int maxNumSamples>
nonstd::span<SampleType> StaticBuffer<SampleType, maxNumChannels, maxNumSamples>::getWriteSpan (int channel) noexcept
{
    hasBeenCleared = false;
    return { channelPointers[(size_t) channel], (size_t) currentNumSamples };
}

template <typename SampleType, int maxNumChannels, int maxNumSamples>
nonstd::span<const SampleType> StaticBuffer<SampleType, maxNumChannels, maxNumSamples>::getReadSpan (int channel) const noexcept
{
    return { channelPointers[(size_t) channel], (size_t) currentNumSamples };
}

template <typename SampleType, int maxNumChannels, int maxNumSamples>
SampleType** StaticBuffer<SampleType, maxNumChannels, maxNumSamples>::getArrayOfWritePointers() noexcept
{
    hasBeenCleared = false;
    return channelPointers.data();
}

template <typename SampleType, int maxNumChannels, int maxNumSamples>
const SampleType** StaticBuffer<SampleType, maxNumChannels, maxNumSamples>::getArrayOfReadPointers() const noexcept
{
    return const_cast<const SampleType**> (channelPointers.data()); // NOSONAR (using const_cast to be more strict)
}

#if CHOWDSP_USING_JUCE
template <typename SampleType, int maxNumChannels, int maxNumSamples>
juce::AudioBuffer<SampleType> StaticBuffer<SampleType, maxNumChannels, maxNumSamples>::toAudioBuffer()
{
    return { getArrayOfWritePointers(), currentNumChannels, currentNumSamples };
}

template <typename SampleType, int maxNumChannels, int maxNumSamples>
juce::AudioBuffer<SampleType> StaticBuffer<SampleType, maxNumChannels, maxNumSamples>::toAudioBuffer() const
{
    return { const_cast<SampleType* const*> (getArrayOfReadPointers()), currentNumChannels, currentNumSamples }; // NOSONAR
}

#if JUCE_MODULE_AVAILABLE_juce_dsp
template <typename SampleType, int maxNumChannels, int maxNumSamples>
AudioBlock<SampleType> StaticBuffer<SampleType, maxNumChannels, maxNumSamples>::toAudioBlock()
{
    return { getArrayOfWritePointers(), (size_t) currentNumChannels, (size_t) currentNumSamples };
}

template <typename SampleType, int maxNumChannels, int maxNumSamples>
AudioBlock<const SampleType> StaticBuffer<SampleType, maxNumChannels, maxNumSamples>::toAudioBlock() const
{
    return { getArrayOfReadPointers(), (size_t) currentNumChannels, (size_t) currentNumSamples };
}
#endif
#endif

template <typename SampleType, int maxNumChannels, int maxNumSamples>
void StaticBuffer<SampleType, maxNumChannels, maxNumSamples>::clear() noexcept
{
    if (hasBeenCleared)
        return;

    buffer_detail::clear (channelPointers.data(), 0, currentNumChannels, 0, currentNumSamples);
    hasBeenCleared = true;
}
} // namespace chowdsp
