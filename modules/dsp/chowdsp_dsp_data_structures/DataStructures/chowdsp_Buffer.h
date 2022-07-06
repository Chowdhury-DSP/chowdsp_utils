#pragma once

namespace chowdsp
{
/**
 * An audio sample buffer that allocates its own memory.
 */
template <typename SampleType>
class Buffer
{
public:
    /** Default Constructor */
    Buffer();

    /** Constructs the buffer with a given size. */
    Buffer (int numChannels, int numSamples);

    /** Move constructor */
    Buffer(Buffer<SampleType>&&) noexcept = default;

    /** Move assignment */
    Buffer<SampleType>& operator= (Buffer<SampleType>&&) noexcept = default;

    void setMaxSize (int numChannels, int numSamples);
    void setCurrentSize (int numChannels, int numSamples) noexcept;

    void clear() noexcept;

    [[nodiscard]] int getNumChannels() const noexcept { return currentNumChannels; }
    [[nodiscard]] int getNumSamples() const noexcept { return currentNumSamples; }

    SampleType* getWritePointer (int channel) noexcept;
    const SampleType* getReadPointer (int channel) const noexcept;

    SampleType** getArrayofWritePointers() noexcept;
    const SampleType** getArrayOfReadPointers() const noexcept;

private:
    template <typename T = SampleType>
    std::enable_if_t<std::is_floating_point_v<T>, void>
        clearInternal (int startChannel, int endChannel, int startSample, int endSample) noexcept;

    template <typename T = SampleType>
    std::enable_if_t<SampleTypeHelpers::IsSIMDRegister<T>, void>
        clearInternal (int startChannel, int endChannel, int startSample, int endSample) noexcept;

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
