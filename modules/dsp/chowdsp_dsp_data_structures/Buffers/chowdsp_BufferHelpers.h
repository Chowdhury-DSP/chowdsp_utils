#pragma once

namespace chowdsp
{
#ifndef DOXYGEN
namespace detail
{
    template <typename BufferType>
    struct ConstBuffer;

    template <typename SampleType>
    struct ConstBuffer<Buffer<SampleType>>
    {
        using type = const Buffer<SampleType>;
    };

    template <typename SampleType, int numChannels, int numSamples>
    struct ConstBuffer<StaticBuffer<SampleType, numChannels, numSamples>>
    {
        using type = const StaticBuffer<SampleType, numChannels, numSamples>;
    };

    template <typename SampleType>
    struct ConstBuffer<BufferView<SampleType>>
    {
        using type = const BufferView<const SampleType>;
    };

    template <typename SampleType>
    struct ConstBuffer<const BufferView<SampleType>>
    {
        using type = const BufferView<const SampleType>;
    };

#if CHOWDSP_USING_JUCE
    template <typename SampleType>
    struct ConstBuffer<juce::AudioBuffer<SampleType>>
    {
        using type = const juce::AudioBuffer<SampleType>;
    };
#endif
} // namespace detail
#endif // DOXYGEN

/** Returns true if this buffer contains const data. */
template <typename BufferType>
static constexpr bool IsConstBufferType = std::is_same_v<
    typename detail::ConstBuffer<std::decay_t<BufferType>>::type,
    std::remove_reference_t<BufferType>>;

/** Returns this buffer with const data. */
template <typename SampleType>
static constexpr auto& asConstBuffer (Buffer<SampleType>& buffer)
{
    return std::as_const (buffer);
}

/** Returns this buffer with const data. */
template <typename SampleType, int numChannels, int numSamples>
static constexpr auto& asConstBuffer (StaticBuffer<SampleType, numChannels, numSamples>& buffer)
{
    return std::as_const (buffer);
}

/** Returns this buffer view with const data. */
template <typename SampleType>
static constexpr auto asConstBuffer (BufferView<SampleType>& buffer)
{
    return BufferView<const SampleType> { buffer };
}

/** Returns this buffer view with const data. */
template <typename SampleType>
static constexpr auto asConstBuffer (const BufferView<SampleType>& buffer)
{
    return BufferView<const SampleType> { buffer };
}

#if CHOWDSP_USING_JUCE
/** Returns this buffer with const data. */
template <typename SampleType>
static constexpr auto& asConstBuffer (juce::AudioBuffer<SampleType>& buffer)
{
    return std::as_const (buffer);
}
#endif
} // namespace chowdsp
