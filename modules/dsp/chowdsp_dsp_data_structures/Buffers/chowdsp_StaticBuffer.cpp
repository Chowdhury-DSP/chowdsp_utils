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

template <typename SampleType, int maxNumChannels, int maxNumSamples>
void StaticBuffer<SampleType, maxNumChannels, maxNumSamples>::clear() noexcept
{
    if (hasBeenCleared)
        return;

    buffer_detail::clear (channelPointers.data(), 0, currentNumChannels, 0, currentNumSamples);
    hasBeenCleared = true;
}
} // namespace chowdsp
