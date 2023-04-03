#pragma once

namespace chowdsp
{
/** Iterators for iterating over buffers */
namespace buffer_iters
{
    /** Iterates over a buffer's channels */
    template <typename BufferType>
    constexpr auto channels (BufferType& buffer)
    {
        struct iterator
        {
            BufferType& buffer;
            int channelIndex;
            bool operator!= (const iterator& other) const
            {
                return &buffer != &other.buffer || channelIndex != other.channelIndex;
            }

            void operator++()
            {
                ++channelIndex;
            }

            auto operator*() const
            {
                if constexpr (IsConstBuffer<BufferType>)
                {
#if CHOWDSP_USING_JUCE
                    if constexpr (std::is_same_v<BufferType, const juce::AudioBuffer<float>> || std::is_same_v<BufferType, const juce::AudioBuffer<double>>)
                    {
                        return std::make_tuple (channelIndex,
                                                nonstd::span { buffer.getReadPointer (channelIndex),
                                                               (size_t) buffer.getNumSamples() });
                    }
                    else
#endif
                    {
                        return std::make_tuple (channelIndex, buffer.getReadSpan (channelIndex));
                    }
                }
                else
                {
#if CHOWDSP_USING_JUCE
                    if constexpr (std::is_same_v<BufferType, juce::AudioBuffer<float>> || std::is_same_v<BufferType, juce::AudioBuffer<double>>)
                    {
                        return std::make_tuple (channelIndex,
                                                nonstd::span { buffer.getWritePointer (channelIndex),
                                                               (size_t) buffer.getNumSamples() });
                    }
                    else
#endif
                    {
                        return std::make_tuple (channelIndex, buffer.getWriteSpan (channelIndex));
                    }
                }
            }
        };
        struct iterable_wrapper
        {
            BufferType& buffer;
            auto begin() { return iterator { buffer, 0 }; }
            auto end() { return iterator { buffer, buffer.getNumChannels() }; }
        };
        return iterable_wrapper { buffer };
    }

    /** Iterates over a buffer's channels */
    template <typename SampleType,
              typename = typename std::enable_if_t<std::is_floating_point_v<SampleType> || SampleTypeHelpers::IsSIMDRegister<SampleType>>>
    constexpr auto channels (const BufferView<SampleType>& buffer)
    {
        return channels<const BufferView<SampleType>> (buffer);
    }

    /**
     * Iterates over a buffer in sub-blocks.
     *
     * @tparam subBlockSize The iterator will always supply sub-blocks of this size _or smaller_.
     * @tparam channelWise  If true, the iterator will iterate over the buffer channels as the innermost loop.
     */
    template <int subBlockSize, bool channelWise = false, typename BufferType>
    constexpr auto sub_blocks (BufferType& buffer)
    {
        struct iterator
        {
            BufferType& buffer;
            int samplesRemaining;
            int channelIndex;
            bool operator!= (const iterator& other) const
            {
                if constexpr (channelWise)
                    return &buffer != &other.buffer || samplesRemaining != other.samplesRemaining;
                else
                    return &buffer != &other.buffer || channelIndex != other.channelIndex;
            }

            void operator++()
            {
                if constexpr (channelWise)
                {
                    ++channelIndex;
                    if (channelIndex == buffer.getNumChannels())
                    {
                        samplesRemaining = juce::jmax (samplesRemaining - subBlockSize, 0);
                        if (samplesRemaining > 0)
                            channelIndex = 0;
                    }
                }
                else
                {
                    samplesRemaining = juce::jmax (samplesRemaining - subBlockSize, 0);
                    if (samplesRemaining == 0)
                    {
                        ++channelIndex;
                        if (channelIndex < buffer.getNumChannels())
                            samplesRemaining = buffer.getNumSamples();
                    }
                }
            }

            auto operator*() const
            {
                const auto startSample = buffer.getNumSamples() - samplesRemaining;
                const auto activeSubBlockSize = (size_t) juce::jmin (subBlockSize, samplesRemaining);
                if constexpr (IsConstBuffer<BufferType>)
                {
#if CHOWDSP_USING_JUCE
                    if constexpr (std::is_same_v<BufferType, const juce::AudioBuffer<float>> || std::is_same_v<BufferType, const juce::AudioBuffer<double>>)
                    {
                        return std::make_tuple (channelIndex,
                                                startSample,
                                                nonstd::span {
                                                    buffer.getReadPointer (channelIndex) + startSample,
                                                    activeSubBlockSize });
                    }
                    else
#endif
                    {
                        return std::make_tuple (channelIndex,
                                                startSample,
                                                buffer.getReadSpan (channelIndex)
                                                    .subspan ((size_t) startSample, activeSubBlockSize));
                    }
                }
                else
                {
#if CHOWDSP_USING_JUCE
                    if constexpr (std::is_same_v<BufferType, juce::AudioBuffer<float>> || std::is_same_v<BufferType, juce::AudioBuffer<double>>)
                    {
                        return std::make_tuple (channelIndex,
                                                startSample,
                                                nonstd::span {
                                                    buffer.getWritePointer (channelIndex) + startSample,
                                                    activeSubBlockSize });
                    }
                    else
#endif
                    {
                        return std::make_tuple (channelIndex,
                                                startSample,
                                                buffer.getWriteSpan (channelIndex)
                                                    .subspan ((size_t) startSample, activeSubBlockSize));
                    }
                }
            }
        };
        struct iterable_wrapper
        {
            BufferType& buffer;
            auto begin() { return iterator { buffer, buffer.getNumSamples(), 0 }; }
            auto end() { return iterator { buffer, 0, buffer.getNumChannels() }; }
        };
        return iterable_wrapper { buffer };
    }

    /**
     * Iterates over a buffer in sub-blocks.
     *
     * @tparam subBlockSize The iterator will always supply sub-blocks of this size _or smaller_.
     * @tparam channelWise  If true, the iterator will iterate over the buffer channels as the innermost loop.
     */
    template <int subBlockSize,
              bool channelWise = false,
              typename SampleType,
              typename = typename std::enable_if_t<std::is_floating_point_v<SampleType> || SampleTypeHelpers::IsSIMDRegister<SampleType>>>
    constexpr auto sub_blocks (const BufferView<SampleType>& buffer)
    {
        return sub_blocks<subBlockSize, channelWise, const BufferView<SampleType>> (buffer);
    }

    /** Iterates over a buffer's samples*/
    template <typename BufferType>
    constexpr auto samples (BufferType& buffer)
    {
        struct iterator
        {
            using SampleType = BufferSampleType<std::remove_const_t<BufferType>>;
            using SamplePtrType = typename std::conditional_t<IsConstBuffer<BufferType>, const SampleType*, SampleType*>;

            BufferType& buffer;
            int sampleIndex;

            iterator (BufferType& _buffer, int _sampleIndex)
                : buffer (_buffer),
                  sampleIndex (_sampleIndex),
                  numChannels (buffer.getNumChannels())
            {
                for (int channel { 0 }; channel < numChannels; ++channel)
                {
                    if constexpr (! IsConstBuffer<BufferType>)
                        channelPtrs[channel] = buffer.getWritePointer (channel);
                    else
                        channelPtrs[channel] = buffer.getReadPointer (channel);
                }
            }

            const int numChannels = 0;
            SamplePtrType channelPtrs[CHOWDSP_BUFFER_MAX_NUM_CHANNELS] {};

#if CHOWDSP_NO_XSIMD
            alignas (16) SampleType channelData[CHOWDSP_BUFFER_MAX_NUM_CHANNELS];
#else
            alignas (xsimd::batch<SampleTypeHelpers::NumericType<SampleType>>::arch_type::alignment()) SampleType channelData[CHOWDSP_BUFFER_MAX_NUM_CHANNELS];
#endif

            bool operator!= (const iterator& other) const
            {
                return &buffer != &other.buffer || sampleIndex != other.sampleIndex;
            }

            void operator++()
            {
                if constexpr (! IsConstBuffer<BufferType>)
                {
                    for (int channel { 0 }; channel < numChannels; ++channel)
                        *channelPtrs[channel] = channelData[channel];
                }

                ++sampleIndex;
                for (int channel { 0 }; channel < numChannels; ++channel)
                    channelPtrs[channel]++;
            }

            auto operator*()
            {
                for (int channel { 0 }; channel < numChannels; ++channel)
                    channelData[channel] = *channelPtrs[channel];

                if constexpr (IsConstBuffer<BufferType>)
                {
                    return std::make_tuple (sampleIndex,
                                            nonstd::span<const SampleType> { channelData, (size_t) numChannels });
                }
                else
                {
                    return std::make_tuple (sampleIndex,
                                            nonstd::span<SampleType> { channelData, (size_t) numChannels });
                }
            }
        };

        struct iterable_wrapper
        {
            BufferType& buffer;
            auto begin() { return iterator { buffer, 0 }; }
            auto end() { return iterator { buffer, buffer.getNumSamples() }; }
        };
        return iterable_wrapper { buffer };
    }

} // namespace buffer_iters
} // namespace chowdsp
