#pragma once

namespace chowdsp
{
#ifndef DOXYGEN
namespace buffer_detail
{
    template <typename SampleType>
    std::enable_if_t<std::is_floating_point_v<SampleType>, void>
        clear (SampleType** channelData, int startChannel, int endChannel, int startSample, int endSample) noexcept
    {
        for (int ch = startChannel; ch < endChannel; ++ch)
            juce::FloatVectorOperations::clear (channelData[(size_t) ch] + startSample, endSample - startSample);
    }

    template <typename SampleType>
    std::enable_if_t<SampleTypeHelpers::IsSIMDRegister<SampleType>, void>
        clear (SampleType** channelData, int startChannel, int endChannel, int startSample, int endSample) noexcept
    {
        for (int ch = startChannel; ch < endChannel; ++ch)
            std::fill (channelData[(size_t) ch] + startSample, channelData[(size_t) ch] + endSample, SampleType {});
    }
} // namespace buffer_detail
#endif

/**
 * An audio sample buffer that allocates its own memory.
 */
template <typename SampleType>
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
    Buffer (Buffer<SampleType>&&) noexcept = default;

    /** Move assignment */
    Buffer<SampleType>& operator= (Buffer<SampleType>&&) noexcept = default;

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

    /** Returns the entire buffer as an array of pointers to each channel's data. */
    [[nodiscard]] SampleType** getArrayOfWritePointers() noexcept;

    /** Returns the entire buffer as an array of pointers to each channel's data. */
    [[nodiscard]] const SampleType** getArrayOfReadPointers() const noexcept;

private:
    using Allocator = xsimd::default_allocator<SampleType>;
    using ChannelData = std::vector<SampleType, Allocator>;
    std::vector<ChannelData> rawData;

    int currentNumChannels = 0;
    int currentNumSamples = 0;
    bool hasBeenCleared = true;

    // Assuming we will never need an audio buffer with more than 64 channels.
    // Maybe we'll need to increase this is we're doing high-order ambisonics or something?
    static constexpr int maxNumChannels = 64;
    std::array<SampleType*, (size_t) maxNumChannels> channelPointers {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Buffer)
};
} // namespace chowdsp
