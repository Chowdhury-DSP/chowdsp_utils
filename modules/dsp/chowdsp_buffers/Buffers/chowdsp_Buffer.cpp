#include "chowdsp_Buffer.h"

namespace chowdsp
{
template <typename SampleType, size_t alignment>
Buffer<SampleType, alignment>::Buffer (int numChannels, int numSamples)
{
    setMaxSize (numChannels, numSamples);
}

template <typename SampleType, size_t alignment>
void Buffer<SampleType, alignment>::setMaxSize (int numChannels, int numSamples)
{
    // Make sure we don't have any null internal buffers
    jassert (juce::isPositiveAndNotGreaterThan (numChannels, maxNumChannels));

    numChannels = juce::jmax (numChannels, 1);
    numSamples = juce::jmax (numSamples, 0);

    int numSamplesPadded = numSamples;
#if ! CHOWDSP_NO_XSIMD
    static constexpr auto vec_size = (int) xsimd::batch<SampleType>::size;
    if constexpr (std::is_floating_point_v<SampleType>)
        numSamplesPadded = buffers_detail::ceiling_divide (numSamples, vec_size) * vec_size;
#endif

    rawData.clear();
    hasBeenCleared = true;
    currentNumChannels = 0;
    currentNumSamples = 0;

    rawData.resize ((size_t) numChannels * (size_t) numSamplesPadded, SampleType {});
    std::fill (channelPointers.begin(), channelPointers.end(), nullptr);
    for (int ch = 0; ch < numChannels; ++ch)
        channelPointers[(size_t) ch] = rawData.data() + ch * numSamplesPadded;

    setCurrentSize (numChannels, numSamples);
}

template <typename SampleType, size_t alignment>
void Buffer<SampleType, alignment>::setCurrentSize (int numChannels, int numSamples) noexcept
{
    // trying to set a current size, but we don't have enough memory allocated!
    jassert (numSamples * numChannels <= (int) rawData.size());

    const auto increasingNumChannels = numChannels > currentNumChannels;
    const auto increasingNumSamples = numSamples > currentNumSamples;

    if (increasingNumSamples)
        buffer_detail::clear (channelPointers.data(), 0, currentNumChannels, currentNumSamples, numSamples);

    if (increasingNumChannels)
        buffer_detail::clear (channelPointers.data(), currentNumChannels, numChannels, 0, numSamples);

    currentNumChannels = numChannels;
    currentNumSamples = numSamples;
}

template <typename SampleType, size_t alignment>
SampleType* Buffer<SampleType, alignment>::getWritePointer (int channel) noexcept
{
    hasBeenCleared = false;
    return channelPointers[(size_t) channel];
}

template <typename SampleType, size_t alignment>
const SampleType* Buffer<SampleType, alignment>::getReadPointer (int channel) const noexcept
{
    return channelPointers[(size_t) channel];
}

template <typename SampleType, size_t alignment>
nonstd::span<SampleType> Buffer<SampleType, alignment>::getWriteSpan (int channel) noexcept
{
    hasBeenCleared = false;
    return { channelPointers[(size_t) channel], (size_t) currentNumSamples };
}

template <typename SampleType, size_t alignment>
nonstd::span<const SampleType> Buffer<SampleType, alignment>::getReadSpan (int channel) const noexcept
{
    return { channelPointers[(size_t) channel], (size_t) currentNumSamples };
}

template <typename SampleType, size_t alignment>
SampleType** Buffer<SampleType, alignment>::getArrayOfWritePointers() noexcept
{
    hasBeenCleared = false;
    return channelPointers.data();
}

template <typename SampleType, size_t alignment>
const SampleType** Buffer<SampleType, alignment>::getArrayOfReadPointers() const noexcept
{
    return const_cast<const SampleType**> (channelPointers.data()); // NOSONAR (using const_cast to be more strict)
}

#if CHOWDSP_USING_JUCE
template <typename SampleType, size_t alignment>
template <typename T>
std::enable_if_t<buffer_detail::IsFloatOrDouble<T>, juce::AudioBuffer<SampleType>> Buffer<SampleType, alignment>::toAudioBuffer()
{
    return { getArrayOfWritePointers(), currentNumChannels, currentNumSamples };
}

template <typename SampleType, size_t alignment>
template <typename T>
std::enable_if_t<buffer_detail::IsFloatOrDouble<T>, juce::AudioBuffer<SampleType>> Buffer<SampleType, alignment>::toAudioBuffer() const
{
    return { const_cast<SampleType* const*> (getArrayOfReadPointers()), currentNumChannels, currentNumSamples }; // NOSONAR
}

#if JUCE_MODULE_AVAILABLE_juce_dsp
template <typename SampleType, size_t alignment>
template <typename T>
std::enable_if_t<buffer_detail::IsFloatOrDouble<T>, AudioBlock<SampleType>> Buffer<SampleType, alignment>::toAudioBlock()
{
    return { getArrayOfWritePointers(), (size_t) currentNumChannels, (size_t) currentNumSamples };
}

template <typename SampleType, size_t alignment>
template <typename T>
std::enable_if_t<buffer_detail::IsFloatOrDouble<T>, AudioBlock<const SampleType>> Buffer<SampleType, alignment>::toAudioBlock() const
{
    return { getArrayOfReadPointers(), (size_t) currentNumChannels, (size_t) currentNumSamples };
}
#endif
#endif

template <typename SampleType, size_t alignment>
void Buffer<SampleType, alignment>::clear() noexcept
{
    if (hasBeenCleared)
        return;

    buffer_detail::clear (channelPointers.data(), 0, currentNumChannels, 0, currentNumSamples);
    hasBeenCleared = true;
}

#if CHOWDSP_ALLOW_TEMPLATE_INSTANTIATIONS
template class Buffer<float>;
template class Buffer<double>;
#if CHOWDSP_USING_JUCE
template juce::AudioBuffer<float> Buffer<float>::toAudioBuffer<float>();
template juce::AudioBuffer<double> Buffer<double>::toAudioBuffer<double>();
template juce::AudioBuffer<float> Buffer<float>::toAudioBuffer<float>() const;
template juce::AudioBuffer<double> Buffer<double>::toAudioBuffer<double>() const;
#if JUCE_MODULE_AVAILABLE_juce_dsp
template AudioBlock<float> Buffer<float>::toAudioBlock<float>();
template AudioBlock<double> Buffer<double>::toAudioBlock<double>();
template AudioBlock<const float> Buffer<float>::toAudioBlock<float>() const;
template AudioBlock<const double> Buffer<double>::toAudioBlock<double>() const;
#endif
#endif

#if ! CHOWDSP_NO_XSIMD
template class Buffer<xsimd::batch<float>>;
template class Buffer<xsimd::batch<double>>;
#endif
#endif
} // namespace chowdsp
