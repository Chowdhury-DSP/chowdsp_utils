#include "chowdsp_Buffer.h"

namespace chowdsp
{
template <typename SampleType>
Buffer<SampleType>::Buffer() = default;

template <typename SampleType>
Buffer<SampleType>::Buffer (int numChannels, int numSamples)
{
    setMaxSize (numChannels, numSamples);
}

template <typename SampleType>
void Buffer<SampleType>::setMaxSize (int numChannels, int numSamples)
{
    jassert (juce::isPositiveAndBelow (numChannels, maxNumChannels));
    jassert (numSamples > 0);

    rawData.resize (numChannels, ChannelData (numSamples, SampleType {}));
    hasBeenCleared = true;

    for (int ch = 0; ch < numChannels; ++ch)
    {
        channelPointers[ch] = rawData[(size_t) ch].data();
    }

    setCurrentSize (numChannels, numSamples);
}

template <typename SampleType>
void Buffer<SampleType>::setCurrentSize (int numChannels, int numSamples) noexcept
{
    const auto increasingNumChannels = numChannels > currentNumChannels;
    const auto increasingNumSamples = numSamples > currentNumSamples;

    if (increasingNumSamples)
        clearInternal (0, currentNumChannels, currentNumSamples, numSamples);

    if (increasingNumChannels)
        clearInternal (numChannels, currentNumChannels, 0, numSamples);

    currentNumChannels = numChannels;
    currentNumSamples = numSamples;
}

template <typename SampleType>
SampleType* Buffer<SampleType>::getWritePointer (int channel) noexcept
{
    hasBeenCleared = false;
    return channelPointers[(size_t) channel];
}

template <typename SampleType>
const SampleType* Buffer<SampleType>::getReadPointer (int channel) const noexcept
{
    return channelPointers[(size_t) channel];
}

template <typename SampleType>
SampleType** Buffer<SampleType>::getArrayofWritePointers() noexcept
{
    hasBeenCleared = false;
    return channelPointers.data();
}

template <typename SampleType>
const SampleType** Buffer<SampleType>::getArrayOfReadPointers() const noexcept
{
    return channelPointers.data();
}

template <typename SampleType>
void Buffer<SampleType>::clear() noexcept
{
    if (hasBeenCleared)
        return;

    clearInternal (0, currentNumChannels, 0, currentNumSamples);
    hasBeenCleared = true;
}

template <typename SampleType>
template <typename T>
std::enable_if_t<std::is_floating_point_v<T>, void>
    Buffer<SampleType>::clearInternal (int startChannel, int endChannel, int startSample, int endSample) noexcept
{
    for (int ch = startChannel; ch < endChannel; ++ch)
        juce::FloatVectorOperations::clear (channelPointers[(size_t) ch] + startSample, endSample - startSample);
}

template <typename SampleType>
template <typename T>
std::enable_if_t<SampleTypeHelpers::IsSIMDRegister<T>, void>
    Buffer<SampleType>::clearInternal (int startChannel, int endChannel, int startSample, int endSample) noexcept
{
    for (int ch = startChannel; ch < endChannel; ++ch)
        std::fill (channelPointers[(size_t) ch] + startSample, channelPointers[(size_t) ch] + endSample, SampleType{});
}
} // namespace chowdsp
