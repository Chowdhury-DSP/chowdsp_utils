#pragma once

namespace chowdsp
{
#ifndef DOXYGEN
namespace buffer_detail
{
    template <typename ElementType, size_t arraySize, size_t alignment = SIMDUtils::defaultSIMDAlignment>
    struct AlignedArray
    {
        //        ElementType& operator[] (size_t index) noexcept { return array[index]; }
        //        const ElementType& operator[] (size_t index) const noexcept { return array[index]; }

        ElementType* data() noexcept { return array; }
        const ElementType* data() const noexcept { return array; }

        void clear() { std::fill (std::begin (array), std::end (array), ElementType {}); }
        static constexpr int size() noexcept { return arraySize; }

    private:
        alignas (alignment) ElementType array[arraySize] {};
    };
} // namespace buffer_detail
#endif

/**
 * An audio sample buffer with statically allocated memory.
 */
template <typename SampleType, int maxNumChannels, int maxNumSamples>
class StaticBuffer
{
public:
    /** The sample type used by the buffer */
    using Type = SampleType;

    /** Default Constructor */
    StaticBuffer() = default;

    /** Constructs the buffer with a given size. */
    StaticBuffer (int numChannels, int numSamples);

    /** Move constructor */
    StaticBuffer (StaticBuffer<SampleType, maxNumChannels, maxNumSamples>&&) noexcept = default;

    /** Move assignment */
    StaticBuffer<SampleType, maxNumChannels, maxNumSamples>& operator= (StaticBuffer<SampleType, maxNumChannels, maxNumSamples>&&) noexcept = default;

    /**
     * This method doesn't really do anything, but exists for easy
     * compatibility with chowdsp::Buffer.
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
    int currentNumChannels = 0;
    int currentNumSamples = 0;
    bool hasBeenCleared = true;

    std::array<buffer_detail::AlignedArray<SampleType, (size_t) maxNumSamples>, (size_t) maxNumChannels> channelData {};
    std::array<SampleType*, (size_t) maxNumChannels> channelPointers {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StaticBuffer)
};
} // namespace chowdsp

#include "chowdsp_StaticBuffer.cpp"
