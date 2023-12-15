#pragma once

namespace chowdsp
{
/** Iterators for iterating over buffers */
namespace buffer_iters
{
#ifndef DOXYGEN
    namespace detail
    {
        template <typename BufferType>
        auto getWriteSpan (BufferType& buffer, int channelIndex)
        {
#if CHOWDSP_USING_JUCE
            if constexpr (std::is_same_v<BufferType, juce::AudioBuffer<float>> || std::is_same_v<BufferType, juce::AudioBuffer<double>>)
                return nonstd::span { buffer.getWritePointer (channelIndex), (size_t) buffer.getNumSamples() };
            else
#endif
                return buffer.getWriteSpan (channelIndex);
        }

        template <typename BufferType>
        auto getWriteSubSpan (BufferType& buffer, int channelIndex, int start, size_t size)
        {
#if CHOWDSP_USING_JUCE
            if constexpr (std::is_same_v<BufferType, juce::AudioBuffer<float>> || std::is_same_v<BufferType, juce::AudioBuffer<double>>)
                return nonstd::span { buffer.getWritePointer (channelIndex) + start, size };
            else
#endif
                return buffer.getWriteSpan (channelIndex).subspan ((size_t) start, size);
        }

        template <typename BufferType>
        auto getReadSpan (BufferType& buffer, int channelIndex)
        {
#if CHOWDSP_USING_JUCE
            if constexpr (std::is_same_v<BufferType, const juce::AudioBuffer<float>> || std::is_same_v<BufferType, const juce::AudioBuffer<double>>)
                return nonstd::span { buffer.getReadPointer (channelIndex), (size_t) buffer.getNumSamples() };
            else
#endif
                return buffer.getReadSpan (channelIndex);
        }

        template <typename BufferType>
        auto getReadSubSpan (BufferType& buffer, int channelIndex, int start, size_t size)
        {
#if CHOWDSP_USING_JUCE
            if constexpr (std::is_same_v<BufferType, const juce::AudioBuffer<float>> || std::is_same_v<BufferType, const juce::AudioBuffer<double>>)
                return nonstd::span { buffer.getReadPointer (channelIndex) + start, size };
            else
#endif
                return buffer.getReadSpan (channelIndex).subspan ((size_t) start, size);
        }

        template <typename BufferType>
        auto getSpan (BufferType& buffer, int channelIndex)
        {
            if constexpr (IsConstBuffer<BufferType>)
                return getReadSpan (buffer, channelIndex);
            else
                return getWriteSpan (buffer, channelIndex);
        }

        template <typename BufferType>
        auto getSubSpan (BufferType& buffer, int channelIndex, int start, size_t size)
        {
            if constexpr (IsConstBuffer<BufferType>)
                return getReadSubSpan (buffer, channelIndex, start, size);
            else
                return getWriteSubSpan (buffer, channelIndex, start, size);
        }
    } // namespace detail
#endif

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
                return std::make_tuple (channelIndex, detail::getSpan (buffer, channelIndex));
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
                return std::make_tuple (channelIndex,
                                        startSample,
                                        detail::getSubSpan (buffer, channelIndex, startSample, activeSubBlockSize));
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

    /** Iterates over a buffer's samples*/
    template <typename BufferType>
    constexpr auto samples (BufferType& buffer)
    {
        struct iterator
        {
            using SampleType = BufferSampleType<std::remove_const_t<BufferType>>;
            using SamplePtrType = std::conditional_t<IsConstBuffer<BufferType>, const SampleType*, SampleType*>;

            BufferType& buffer;
            int sampleIndex {};

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
            alignas (16) SampleType channelData[CHOWDSP_BUFFER_MAX_NUM_CHANNELS] {};
#else
            alignas (xsimd::batch<SampleTypeHelpers::NumericType<SampleType>>::arch_type::alignment()) SampleType channelData[CHOWDSP_BUFFER_MAX_NUM_CHANNELS] {};
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
                    ++channelPtrs[channel];
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

    /** Iterates over a buffer's channels */
    template <typename BufferType1, typename BufferType2>
    constexpr auto zip_channels (BufferType1& buffer1, BufferType2& buffer2)
    {
        struct iterator
        {
            BufferType1& buffer1;
            BufferType2& buffer2;
            int channelIndex;
            bool operator!= (const iterator& other) const
            {
                return &buffer1 != &other.buffer1 || &buffer2 != &other.buffer2 || channelIndex != other.channelIndex;
            }

            void operator++()
            {
                ++channelIndex;
            }

            auto operator*() const
            {
                return std::make_tuple (channelIndex,
                                        detail::getSpan (buffer1, channelIndex),
                                        detail::getSpan (buffer2, channelIndex));
            }
        };
        struct iterable_wrapper
        {
            BufferType1& buffer1;
            BufferType2& buffer2;
            auto begin() { return iterator { buffer1, buffer2, 0 }; }
            auto end() { return iterator { buffer1,
                                           buffer2,
                                           std::min (buffer1.getNumChannels(), buffer2.getNumChannels()) }; }
        };
        return iterable_wrapper { buffer1, buffer2 };
    }
} // namespace buffer_iters
} // namespace chowdsp
