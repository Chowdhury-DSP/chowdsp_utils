#pragma once

#if JUCE_MODULE_AVAILABLE_chowdsp_data_structures
#include <chowdsp_data_structures/chowdsp_data_structures.h>
#endif

namespace chowdsp
{
/**
 * A "view" into an audio buffer that has already had its memory
 * allocated. Can be constructed from a chowdsp::Buffer,
 * juce::AudioBuffer, juce::dsp::AudioBlock, or raw memory.
 */
template <typename SampleType>
class BufferView
{
public:
    /** The sample type used by the buffer */
    using Type = SampleType;

    BufferView() = default;

    BufferView& operator= (const BufferView&) = default;
    BufferView (BufferView&&) noexcept = default;
    BufferView& operator= (BufferView&&) noexcept = default;

    BufferView (SampleType* const* data, int dataNumChannels, int dataNumSamples, int sampleOffset = 0) : numChannels (dataNumChannels),
                                                                                                          numSamples (dataNumSamples)
    {
        initialise (data, sampleOffset);
    }

    template <typename T = SampleType, std::enable_if_t<std::is_const_v<T>>* = nullptr>
    BufferView (const SampleType* const* data, int dataNumChannels, int dataNumSamples, int sampleOffset = 0) : numChannels (dataNumChannels),
                                                                                                                numSamples (dataNumSamples)
    {
        initialise (data, sampleOffset);
    }

    BufferView (SampleType* data, int dataNumSamples, int sampleOffset = 0) : numSamples (dataNumSamples)
    {
        initialise (&data, sampleOffset);
    }

    template <typename T = SampleType, std::enable_if_t<std::is_const_v<T>>* = nullptr>
    BufferView (const SampleType* data, int dataNumSamples, int sampleOffset = 0) : numSamples (dataNumSamples)
    {
        initialise (&data, sampleOffset);
    }

    BufferView (Buffer<std::remove_const_t<SampleType>>& buffer, // NOLINT(google-explicit-constructor): we want to be able to do implicit construction
                int sampleOffset = 0,
                int bufferNumSamples = -1,
                int startChannel = 0,
                int bufferNumChannels = -1)
        : numChannels (bufferNumChannels < 0 ? (buffer.getNumChannels() - startChannel) : bufferNumChannels),
          numSamples (bufferNumSamples < 0 ? (buffer.getNumSamples() - sampleOffset) : bufferNumSamples)
    {
        jassert (buffer.getNumChannels() >= startChannel + numChannels);
        jassert (buffer.getNumSamples() >= sampleOffset + numSamples);
        initialise (buffer.getArrayOfWritePointers(), sampleOffset, startChannel);
    }

    template <typename T = SampleType, std::enable_if_t<std::is_const_v<T>>* = nullptr>
    BufferView (const Buffer<std::remove_const_t<SampleType>>& buffer, // NOLINT(google-explicit-constructor): we want to be able to do implicit construction
                int sampleOffset = 0,
                int bufferNumSamples = -1,
                int startChannel = 0,
                int bufferNumChannels = -1)
        : numChannels (bufferNumChannels < 0 ? (buffer.getNumChannels() - startChannel) : bufferNumChannels),
          numSamples (bufferNumSamples < 0 ? (buffer.getNumSamples() - sampleOffset) : bufferNumSamples)
    {
        jassert (buffer.getNumChannels() >= startChannel + numChannels);
        jassert (buffer.getNumSamples() >= sampleOffset + numSamples);
        initialise (buffer.getArrayOfReadPointers(), sampleOffset, startChannel);
    }

    template <int maxNumChannels, int maxNumSamples>
    BufferView (StaticBuffer<std::remove_const_t<SampleType>, maxNumChannels, maxNumSamples>& buffer, // NOLINT(google-explicit-constructor): we want to be able to do implicit construction
                int sampleOffset = 0,
                int bufferNumSamples = -1,
                int startChannel = 0,
                int bufferNumChannels = -1)
        : numChannels (bufferNumChannels < 0 ? (buffer.getNumChannels() - startChannel) : bufferNumChannels),
          numSamples (bufferNumSamples < 0 ? (buffer.getNumSamples() - sampleOffset) : bufferNumSamples)
    {
        jassert (buffer.getNumChannels() >= startChannel + numChannels);
        jassert (buffer.getNumSamples() >= sampleOffset + numSamples);
        initialise (buffer.getArrayOfWritePointers(), sampleOffset, startChannel);
    }

    template <int maxNumChannels, int maxNumSamples, typename T = SampleType, std::enable_if_t<std::is_const_v<T>>* = nullptr>
    BufferView (const StaticBuffer<std::remove_const_t<SampleType>, maxNumChannels, maxNumSamples>& buffer, // NOLINT(google-explicit-constructor): we want to be able to do implicit construction
                int sampleOffset = 0,
                int bufferNumSamples = -1,
                int startChannel = 0,
                int bufferNumChannels = -1)
        : numChannels (bufferNumChannels < 0 ? (buffer.getNumChannels() - startChannel) : bufferNumChannels),
          numSamples (bufferNumSamples < 0 ? (buffer.getNumSamples() - sampleOffset) : bufferNumSamples)
    {
        jassert (buffer.getNumChannels() >= startChannel + numChannels);
        jassert (buffer.getNumSamples() >= sampleOffset + numSamples);
        initialise (buffer.getArrayOfReadPointers(), sampleOffset, startChannel);
    }

    BufferView (const BufferView<std::remove_const_t<SampleType>>& buffer, // NOLINT(google-explicit-constructor): we want to be able to do implicit construction
                int sampleOffset = 0,
                int bufferNumSamples = -1,
                int startChannel = 0,
                int bufferNumChannels = -1)
        : numChannels (bufferNumChannels < 0 ? (buffer.getNumChannels() - startChannel) : bufferNumChannels),
          numSamples (bufferNumSamples < 0 ? (buffer.getNumSamples() - sampleOffset) : bufferNumSamples)
    {
        jassert (buffer.getNumChannels() >= startChannel + numChannels);
        jassert (buffer.getNumSamples() >= sampleOffset + numSamples);
        initialise (buffer.getArrayOfWritePointers(), sampleOffset, startChannel);
    }

    template <typename T = SampleType, std::enable_if_t<std::is_const_v<T>>* = nullptr>
    BufferView (const BufferView<const SampleType>& buffer, // NOLINT(google-explicit-constructor): we want to be able to do implicit construction
                int sampleOffset = 0,
                int bufferNumSamples = -1,
                int startChannel = 0,
                int bufferNumChannels = -1)
        : numChannels (bufferNumChannels < 0 ? (buffer.getNumChannels() - startChannel) : bufferNumChannels),
          numSamples (bufferNumSamples < 0 ? (buffer.getNumSamples() - sampleOffset) : bufferNumSamples)
    {
        jassert (buffer.getNumChannels() >= startChannel + numChannels);
        jassert (buffer.getNumSamples() >= sampleOffset + numSamples);
        initialise (buffer.getArrayOfReadPointers(), sampleOffset, startChannel);
    }

#if CHOWDSP_USING_JUCE
    BufferView (juce::AudioBuffer<std::remove_const_t<SampleType>>& buffer, // NOLINT(google-explicit-constructor): we want to be able to do implicit construction
                int sampleOffset = 0,
                int bufferNumSamples = -1,
                int startChannel = 0,
                int bufferNumChannels = -1)
        : numChannels (bufferNumChannels < 0 ? (buffer.getNumChannels() - startChannel) : bufferNumChannels),
          numSamples (bufferNumSamples < 0 ? (buffer.getNumSamples() - sampleOffset) : bufferNumSamples)
    {
        initialise (buffer.getArrayOfWritePointers(), sampleOffset, startChannel);
    }

    template <typename T = SampleType, std::enable_if_t<std::is_const_v<T>>* = nullptr>
    BufferView (const juce::AudioBuffer<std::remove_const_t<SampleType>>& buffer, // NOLINT(google-explicit-constructor): we want to be able to do implicit construction
                int sampleOffset = 0,
                int bufferNumSamples = -1,
                int startChannel = 0,
                int bufferNumChannels = -1)
        : numChannels (bufferNumChannels < 0 ? (buffer.getNumChannels() - startChannel) : bufferNumChannels),
          numSamples (bufferNumSamples < 0 ? (buffer.getNumSamples() - sampleOffset) : bufferNumSamples)
    {
        initialise (buffer.getArrayOfReadPointers(), sampleOffset, startChannel);
    }

    /** Constructs a juce::AudioBuffer from this BufferView */
    [[nodiscard]] juce::AudioBuffer<SampleType> toAudioBuffer() const noexcept
    {
        return { channelPointers.data(), numChannels, numSamples };
    }

#if JUCE_MODULE_AVAILABLE_juce_dsp
    template <typename T = SampleType, std::enable_if_t<std::is_floating_point_v<T>>* = nullptr>
    BufferView (const AudioBlock<std::remove_const_t<SampleType>>& block, // NOLINT(google-explicit-constructor): we want to be able to do implicit construction
                int sampleOffset = 0,
                int bufferNumSamples = -1,
                int startChannel = 0,
                int bufferNumChannels = -1)
        : numChannels (bufferNumChannels < 0 ? ((int) block.getNumChannels() - startChannel) : bufferNumChannels),
          numSamples (bufferNumSamples < 0 ? ((int) block.getNumSamples() - sampleOffset) : bufferNumSamples)
    {
        for (size_t ch = 0; ch < (size_t) numChannels; ++ch)
            channelPointers[ch] = block.getChannelPointer (ch + (size_t) startChannel) + sampleOffset;
    }

    template <typename T = SampleType, std::enable_if_t<std::is_const_v<T> && std::is_floating_point_v<T>>* = nullptr>
    BufferView (const AudioBlock<const SampleType>& block, // NOLINT(google-explicit-constructor): we want to be able to do implicit construction
                int sampleOffset = 0,
                int bufferNumSamples = -1,
                int startChannel = 0,
                int bufferNumChannels = -1)
        : numChannels (bufferNumChannels < 0 ? ((int) block.getNumChannels() - startChannel) : bufferNumChannels),
          numSamples (bufferNumSamples < 0 ? ((int) block.getNumSamples() - sampleOffset) : bufferNumSamples)
    {
        for (size_t ch = 0; ch < (size_t) numChannels; ++ch)
            channelPointers[ch] = block.getChannelPointer (ch + (size_t) startChannel) + sampleOffset;
    }

    /** Constructs a juce::dsp::AudioBlock from this BufferView */
    [[nodiscard]] AudioBlock<SampleType> toAudioBlock() const noexcept
    {
        return { channelPointers.data(), (size_t) numChannels, (size_t) numSamples };
    }
#endif
#endif

    /** Clears memory within the buffer view. */
    template <typename T = SampleType>
    std::enable_if_t<! std::is_const_v<T>, void> clear() const noexcept
    {
        buffer_detail::clear (const_cast<SampleType**> (channelPointers.data()), 0, numChannels, 0, numSamples);
    }

    /** Returns the number of channels in the buffer view. */
    [[nodiscard]] int getNumChannels() const noexcept { return numChannels; }

    /** Returns the number of samples in the buffer view. */
    [[nodiscard]] int getNumSamples() const noexcept { return numSamples; }

    /** Returns a pointer which can be used to write to a single channel of the buffer view. */
    template <typename T = SampleType>
    [[nodiscard]] std::enable_if_t<! std::is_const_v<T>, SampleType*> getWritePointer (int channel) const noexcept
    {
        return channelPointers[(size_t) channel];
    }

    /** Returns a pointer which can be used to read from a single channel of the buffer view. */
    [[nodiscard]] const SampleType* getReadPointer (int channel) const noexcept { return channelPointers[(size_t) channel]; }

    /** Returns a span which can be used to write to a single channel of the buffer view. */
    template <typename T = SampleType>
    [[nodiscard]] std::enable_if_t<! std::is_const_v<T>, nonstd::span<SampleType>> getWriteSpan (int channel) const noexcept
    {
        return { channelPointers[(size_t) channel], (size_t) numSamples };
    }

    /** Returns a span which can be used to read from a single channel of the buffer view. */
    [[nodiscard]] nonstd::span<const SampleType> getReadSpan (int channel) const noexcept
    {
        return { channelPointers[(size_t) channel], (size_t) numSamples };
    }

    /** Returns the entire buffer view as an array of pointers to each channel's data. */
    template <typename T = SampleType>
    [[nodiscard]] std::enable_if_t<! std::is_const_v<T>, SampleType* const*> getArrayOfWritePointers() const noexcept
    {
        return channelPointers.data();
    }

    /** Returns the entire buffer view as an array of pointers to each channel's data. */
    [[nodiscard]] const SampleType* const* getArrayOfReadPointers() const noexcept { return const_cast<const SampleType* const*> (channelPointers.data()); }

private:
    template <typename T = SampleType>
    std::enable_if_t<! std::is_const_v<T>, void> initialise (SampleType* const* data, int sampleOffset, int startChannel = 0)
    {
        jassert (juce::isPositiveAndNotGreaterThan (numChannels, maxNumChannels));
        jassert (numSamples >= 0);
        for (size_t ch = 0; ch < (size_t) numChannels; ++ch)
            channelPointers[ch] = data[ch + (size_t) startChannel] + sampleOffset;
    }

    template <typename T = SampleType>
    std::enable_if_t<std::is_const_v<T>, void> initialise (const SampleType* const* data, int sampleOffset, int startChannel = 0)
    {
        jassert (juce::isPositiveAndNotGreaterThan (numChannels, maxNumChannels));
        jassert (numSamples >= 0);
        for (size_t ch = 0; ch < (size_t) numChannels; ++ch)
            channelPointers[ch] = data[ch + (size_t) startChannel] + sampleOffset;
    }

    int numChannels = 1;
    int numSamples = 0;

    // Assuming we will never need an audio buffer with more than 64 channels.
    // Maybe we'll need to increase this is we're doing high-order ambisonics or something?
    static constexpr int maxNumChannels = CHOWDSP_BUFFER_MAX_NUM_CHANNELS;
    std::array<SampleType*, (size_t) maxNumChannels> channelPointers {};
};

#ifndef DOXYGEN
namespace detail
{
    template <typename BufferType, typename SampleType>
    struct is_static_buffer
    {
        static constexpr bool value = false;
    };

    template <typename T, int nChannels, int nSamples>
    struct is_static_buffer<StaticBuffer<T, nChannels, nSamples>, T>
    {
        static constexpr bool value = true;
    };

    template <typename BufferType, typename SampleType>
    constexpr auto is_static_buffer_v = is_static_buffer<BufferType, SampleType>::value;

    static_assert (is_static_buffer_v<StaticBuffer<float, 1, 1>, float> == true);
    static_assert (is_static_buffer_v<StaticBuffer<float, 1, 1>, double> != true);
} // namespace detail
#endif // DOXYGEN

/** Template deduction guide for Buffer<float> -> BufferView<float> */
template <typename BufferType,
          typename... Ts,
          typename = std::enable_if_t<
              std::is_same_v<BufferType, Buffer<float>> || detail::is_static_buffer_v<BufferType, float>
#if CHOWDSP_USING_JUCE
              || std::is_same_v<BufferType, juce::AudioBuffer<float>>
#if JUCE_MODULE_AVAILABLE_juce_dsp
              || std::is_same_v<BufferType, juce::dsp::AudioBlock<float>>
#endif
#endif
              >>
BufferView (BufferType&, Ts...) -> BufferView<float>;

/** Template deduction guide for const Buffer<float> -> BufferView<const float> */
template <typename BufferType,
          typename... Ts,
          typename = std::enable_if_t<
              std::is_same_v<BufferType, const Buffer<float>> || (std::is_const_v<BufferType> && detail::is_static_buffer_v<std::remove_const_t<BufferType>, float>)
#if CHOWDSP_USING_JUCE
              || std::is_same_v<BufferType, const juce::AudioBuffer<float>>
#if JUCE_MODULE_AVAILABLE_juce_dsp
              || std::is_same_v<std::remove_const_t<BufferType>, juce::dsp::AudioBlock<const float>>
#endif
#endif
              >>
BufferView (BufferType&, Ts...) -> BufferView<const float>;

/** Template deduction guide for Buffer<double> -> BufferView<const double> */
template <typename BufferType,
          typename... Ts,
          typename = std::enable_if_t<
              std::is_same_v<BufferType, Buffer<double>> || detail::is_static_buffer_v<BufferType, double>
#if CHOWDSP_USING_JUCE
              || std::is_same_v<BufferType, juce::AudioBuffer<double>>
#if JUCE_MODULE_AVAILABLE_juce_dsp
              || std::is_same_v<BufferType, juce::dsp::AudioBlock<double>>
#endif
#endif
              >>
BufferView (BufferType&, Ts...) -> BufferView<double>;

/** Template deduction guide for const Buffer<double> -> BufferView<const double> */
template <typename BufferType,
          typename... Ts,
          typename = std::enable_if_t<
              std::is_same_v<BufferType, const Buffer<double>> || (std::is_const_v<BufferType> && detail::is_static_buffer_v<std::remove_const_t<BufferType>, double>)
#if CHOWDSP_USING_JUCE
              || std::is_same_v<BufferType, const juce::AudioBuffer<double>>
#if JUCE_MODULE_AVAILABLE_juce_dsp
              || std::is_same_v<std::remove_const_t<BufferType>, juce::dsp::AudioBlock<const double>>
#endif
#endif
              >>
BufferView (BufferType&, Ts...) -> BufferView<const double>;

#if ! CHOWDSP_NO_XSIMD
/** Template deduction guide for Buffer<xsimd::batch<float>> -> BufferView<const xsimd::batch<float>> */
template <typename BufferType,
          typename... Ts,
          typename = std::enable_if_t<
              std::is_same_v<BufferType, Buffer<xsimd::batch<float>>> || detail::is_static_buffer_v<BufferType, xsimd::batch<float>>>>
BufferView (BufferType&, Ts...) -> BufferView<xsimd::batch<float>>;

/** Template deduction guide for const Buffer<xsimd::batch<float>> -> BufferView<const xsimd::batch<float>> */
template <typename BufferType,
          typename... Ts,
          typename = std::enable_if_t<
              std::is_same_v<BufferType, const Buffer<xsimd::batch<float>>> || (std::is_const_v<BufferType> && detail::is_static_buffer_v<std::remove_const_t<BufferType>, xsimd::batch<float>>)>>
BufferView (BufferType&, Ts...) -> BufferView<const xsimd::batch<float>>;

/** Template deduction guide for Buffer<xsimd::batch<double>> -> BufferView<const xsimd::batch<double>> */
template <typename BufferType,
          typename... Ts,
          typename = std::enable_if_t<
              std::is_same_v<BufferType, Buffer<xsimd::batch<double>>> || detail::is_static_buffer_v<BufferType, xsimd::batch<double>>>>
BufferView (BufferType&, Ts...) -> BufferView<xsimd::batch<double>>;

/** Template deduction guide for const Buffer<xsimd::batch<double>> -> BufferView<const xsimd::batch<double>> */
template <typename BufferType,
          typename... Ts,
          typename = std::enable_if_t<
              std::is_same_v<BufferType, const Buffer<xsimd::batch<double>>> || (std::is_const_v<BufferType> && detail::is_static_buffer_v<std::remove_const_t<BufferType>, xsimd::batch<double>>)>>
BufferView (BufferType&, Ts...) -> BufferView<const xsimd::batch<double>>;
#endif // ! CHOWDSP_NO_XSIMD

#if JUCE_MODULE_AVAILABLE_chowdsp_data_structures
template <typename T>
BufferView<T> make_temp_buffer (ArenaAllocatorView arena, int num_channels, int num_samples)
{
    int num_samples_padded = num_samples;
#if ! CHOWDSP_NO_XSIMD
    static constexpr auto vec_size = (int) xsimd::batch<T>::size;
    if constexpr (std::is_floating_point_v<T>)
        num_samples_padded = buffers_detail::ceiling_divide (num_samples, vec_size) * vec_size;
#endif

    std::array<T*, CHOWDSP_BUFFER_MAX_NUM_CHANNELS> channel_pointers {};
    for (size_t ch = 0; ch < static_cast<size_t> (num_channels); ++ch)
    {
        channel_pointers[ch] = arena.allocate<T> (num_samples_padded, SIMDUtils::defaultSIMDAlignment);
        jassert (channel_pointers[ch] != nullptr);
    }
    return { channel_pointers.data(), num_channels, num_samples };
}
#endif
} // namespace chowdsp
