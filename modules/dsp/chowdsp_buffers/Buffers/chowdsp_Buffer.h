#pragma once

namespace chowdsp
{
#ifndef DOXYGEN
namespace buffer_detail
{
    template <typename T>
    static constexpr bool IsFloatOrDouble = std::is_same_v<T, float> || std::is_same_v<T, double>;

    template <typename SampleType>
    std::enable_if_t<IsFloatOrDouble<SampleType>, void>
        clear (SampleType** channelData, int startChannel, int endChannel, int startSample, int endSample) noexcept
    {
        for (int ch = startChannel; ch < endChannel; ++ch)
            juce::FloatVectorOperations::clear (channelData[(size_t) ch] + startSample, endSample - startSample);
    }

    template <typename SampleType>
    std::enable_if_t<! IsFloatOrDouble<SampleType>, void>
        clear (SampleType** channelData, int startChannel, int endChannel, int startSample, int endSample) noexcept
    {
        for (int ch = startChannel; ch < endChannel; ++ch)
            std::fill (channelData[(size_t) ch] + startSample, channelData[(size_t) ch] + endSample, SampleType {});
    }
} // namespace buffer_detail
#endif

/**
 * An audio sample buffer that allocates its own memory.
 *
 * By default each buffer channel will be aligned to the appropriate SIMD
 * byte boundary, unless CHOWDSP_NO_XSIMD is enabled. If you would like to
 * provide a custom alignment, you can specify the alignment as the second
 * template argument.
 */
template <typename SampleType, size_t alignment = 0>
class Buffer
{
public:
    /** The sample type used by the buffer */
    using Type = SampleType;

    /** Default Constructor */
    Buffer() = default;

    /** Constructs the buffer with a given size. */
    Buffer (int numChannels, int numSamples);

    /** Move constructor */
    Buffer (Buffer&&) noexcept = default;

    /** Move assignment */
    Buffer& operator= (Buffer&&) noexcept = default;

    /**
     * Sets the maximum size that this buffer can have, and sets the current size as well.
     *
     * This method allocate memory!
     */
    void setMaxSize (int numChannels, int numSamples);

    /**
     * Sets the current size of the buffer. Make sure the requested size
     * is not larger than the maximum size of the buffer!
     *
     * If the requested size is larger than the previous size, any "new" memory
     * will be cleared. Any memory that was previously part of the buffer will
     * not be cleared.
     *
     * This method will never allocate memory!
     */
    void setCurrentSize (int numChannels, int numSamples) noexcept;

    /** Clears memory within the "current size" of the buffer. */
    void clear() noexcept;

    /** Returns the current number of channels in the buffer. */
    [[nodiscard]] int getNumChannels() const noexcept { return currentNumChannels; }

    /** Returns the current number of samples in the buffer. */
    [[nodiscard]] int getNumSamples() const noexcept { return currentNumSamples; }

    /** Returns a pointer which can be used to write to a single channel of the buffer. */
    [[nodiscard]] SampleType* getWritePointer (int channel) noexcept;

    /** Returns a pointer which can be used to read from a single channel of the buffer. */
    [[nodiscard]] const SampleType* getReadPointer (int channel) const noexcept;

    /** Returns a span which can be used to write to a single channel of the buffer. */
    [[nodiscard]] nonstd::span<SampleType> getWriteSpan (int channel) noexcept;

    /** Returns a span which can be used to read from a single channel of the buffer. */
    [[nodiscard]] nonstd::span<const SampleType> getReadSpan (int channel) const noexcept;

    /** Returns the entire buffer as an array of pointers to each channel's data. */
    [[nodiscard]] SampleType** getArrayOfWritePointers() noexcept;

    /** Returns the entire buffer as an array of pointers to each channel's data. */
    [[nodiscard]] const SampleType** getArrayOfReadPointers() const noexcept;

#if CHOWDSP_USING_JUCE
    /** Constructs a juce::AudioBuffer from the data in this buffer */
    template <typename T = SampleType>
    [[nodiscard]] std::enable_if_t<buffer_detail::IsFloatOrDouble<T>, juce::AudioBuffer<SampleType>> toAudioBuffer();

    /** Constructs a juce::AudioBuffer from the data in this buffer */
    template <typename T = SampleType>
    [[nodiscard]] std::enable_if_t<buffer_detail::IsFloatOrDouble<T>, juce::AudioBuffer<SampleType>> toAudioBuffer() const;

#if JUCE_MODULE_AVAILABLE_juce_dsp
    /** Constructs a juce::dsp::AudioBlock from the data in this buffer */
    template <typename T = SampleType>
    [[nodiscard]] std::enable_if_t<buffer_detail::IsFloatOrDouble<T>, AudioBlock<SampleType>> toAudioBlock();

    /** Constructs a juce::dsp::AudioBlock from the data in this buffer */
    template <typename T = SampleType>
    [[nodiscard]] std::enable_if_t<buffer_detail::IsFloatOrDouble<T>, AudioBlock<const SampleType>> toAudioBlock() const;
#endif
#endif

private:
#if ! CHOWDSP_NO_XSIMD
    using Allocator = std::conditional_t<alignment == 0,
                                         xsimd::default_allocator<SampleType>,
                                         xsimd::aligned_allocator<SampleType, alignment>>;
#else
    using Allocator = std::allocator<SampleType>;
#endif
    std::vector<SampleType, Allocator> rawData {};

    int currentNumChannels = 0;
    int currentNumSamples = 0;
    bool hasBeenCleared = true;

    static constexpr int maxNumChannels = CHOWDSP_BUFFER_MAX_NUM_CHANNELS;
    std::array<SampleType*, (size_t) maxNumChannels> channelPointers {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Buffer)
};
} // namespace chowdsp
