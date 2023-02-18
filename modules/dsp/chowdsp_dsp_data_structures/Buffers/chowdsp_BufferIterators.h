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
                if constexpr (IsConstBufferType<BufferType>)
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
    template <typename SampleType>
    constexpr auto channels (const BufferView<SampleType>& buffer)
    {
        return channels<const BufferView<SampleType>> (buffer);
    }

    /**
     * Iterates over a buffer in sub-blocks.
     *
     * @tparam subBlockSize The iterator will always supply sub-blocks of this size _or smaller_.
     * @tparam channelWise  If true, the iterator will iterate over the buffer channels first.
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
                if constexpr (IsConstBufferType<BufferType>)
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
     * @tparam channelWise  If true, the iterator will iterate over the buffer channels first.
     */
    template <int subBlockSize, bool channelWise = false, typename SampleType>
    constexpr auto sub_blocks (const BufferView<SampleType>& buffer)
    {
        return sub_blocks<subBlockSize, channelWise, const BufferView<SampleType>> (buffer);
    }
} // namespace buffer_iters

} // namespace chowdsp
