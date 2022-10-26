#include "chowdsp_Buffer.h"

namespace chowdsp
{
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

    rawData.clear();
    hasBeenCleared = true;
    currentNumChannels = 0;
    currentNumSamples = 0;

    rawData.resize ((size_t) numChannels, ChannelData ((size_t) numSamples, SampleType {}));
    std::fill (channelPointers.begin(), channelPointers.end(), nullptr);
    for (int ch = 0; ch < numChannels; ++ch)
        channelPointers[(size_t) ch] = rawData[(size_t) ch].data();

    setCurrentSize (numChannels, numSamples);
}

template <typename SampleType>
void Buffer<SampleType>::setCurrentSize (int numChannels, int numSamples) noexcept
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
SampleType** Buffer<SampleType>::getArrayOfWritePointers() noexcept
{
    hasBeenCleared = false;
    return channelPointers.data();
}

template <typename SampleType>
const SampleType** Buffer<SampleType>::getArrayOfReadPointers() const noexcept
{
    return const_cast<const SampleType**> (channelPointers.data()); // NOSONAR (using const_cast to be more strict)
}

template <typename SampleType>
void Buffer<SampleType>::clear() noexcept
{
    if (hasBeenCleared)
        return;

    buffer_detail::clear (channelPointers.data(), 0, currentNumChannels, 0, currentNumSamples);
    hasBeenCleared = true;
}

template class Buffer<float>;
template class Buffer<double>;
#if ! CHOWDSP_NO_XSIMD
template class Buffer<xsimd::batch<float>>;
template class Buffer<xsimd::batch<double>>;
#endif
} // namespace chowdsp
