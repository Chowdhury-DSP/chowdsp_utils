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

    /**
     * sample type helper
     *
     * @TODO (Jatin): this is currently a duplicate of the same class in chowdsp::BufferMath::detail
     */
    namespace sample_type
    {
        template <typename BufferType>
        struct BufferSampleTypeHelper
        {
            using Type = std::remove_const_t<typename BufferType::Type>;
        };

#if CHOWDSP_USING_JUCE
        template <>
        struct BufferSampleTypeHelper<juce::AudioBuffer<float>>
        {
            using Type = float;
        };

        template <>
        struct BufferSampleTypeHelper<juce::AudioBuffer<double>>
        {
            using Type = double;
        };
#endif

        /** Template helper for getting the sample type from a buffer. */
        template <typename BufferType>
        using BufferSampleType = typename BufferSampleTypeHelper<BufferType>::Type;
    } // namespace sample_type

    /** Iterates over a buffer's samples*/
    template <typename BufferType>
    constexpr auto samples (BufferType& buffer)
    {
        struct iterator
        {
            using SampleType = sample_type::BufferSampleType<std::remove_const_t<BufferType>>;
            using SamplePtrType = typename std::conditional_t<IsConstBufferType<BufferType>, const SampleType*, SampleType*>;

            BufferType& buffer;
            int sampleIndex;
            SamplePtrType channelData[CHOWDSP_BUFFER_MAX_NUM_CHANNELS];

            bool operator!= (const iterator& other) const
            {
                return &buffer != &other.buffer || sampleIndex != other.sampleIndex;
            }

            void operator++()
            {
                ++sampleIndex;
            }

            auto operator*()
            {
                if constexpr (IsConstBufferType<BufferType>)
                {
                    for (int channel { 0 }; channel < buffer.getNumChannels(); channel++)
                    {
                        channelData[channel] = &buffer.getReadPointer (channel)[sampleIndex];
                    }

                    auto channelSpan = nonstd::span { std::as_const (channelData), (size_t) buffer.getNumChannels() };

                    return std::make_tuple (sampleIndex, channelSpan);
                }
                else
                {
                    for (int channel { 0 }; channel < buffer.getNumChannels(); channel++)
                    {
                        channelData[channel] = &buffer.getWritePointer (channel)[sampleIndex];
                    }

                    auto channelSpan = nonstd::span { std::as_const (channelData), (size_t) buffer.getNumChannels() };

                    return std::make_tuple (sampleIndex, channelSpan);
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
