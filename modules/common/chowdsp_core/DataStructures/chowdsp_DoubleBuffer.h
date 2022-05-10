#pragma once

#include <algorithm>
#include <type_traits>
#include <vector>

#if JUCE_MODULE_AVAILABLE_juce_audio_basics
#include <juce_audio_basics/juce_audio_basics.h>
#endif

namespace chowdsp
{
/**
 * Circular double buffer. Similar to a plain circular buffer, except that
 * a contiguous segment of data is always available. The cost is that the
 * buffer takes up twice as much memory and writing to the buffer takes twice
 * as long (but is still pretty fast).
 */
template <typename T>
class DoubleBuffer
{
public:
    /** Default Constructor */
    DoubleBuffer() = default;

    /** Constructor with initial size and value */
    explicit DoubleBuffer (int initialSize, T initialValue = T (0))
    {
        resize (initialSize, initialValue);
    }

    /** Returns the current size of the buffer */
    [[nodiscard]] int size() const noexcept { return (int) internal.size() / 2; }

    /** Returns the current position of the buffer's write pointer */
    [[nodiscard]] int getWritePointer() const noexcept { return writePointer; }

    /**
     * Allows the buffer to be resized, with a given default value.
     * It is worth noting that this is the only DoubleBuffer method that allocates memory.
     */
    void resize (int size, T defaultValue = T (0))
    {
        internal.resize ((size_t) size * 2, defaultValue);
        writePointer = 0;
    }

    /** Clear's the DoubleBuffer's data */
    void clear()
    {
        fill ((T) 0);
    }

    /** Fill's the DoubleBuffer with a single value */
    void fill (T value)
    {
        std::fill (internal.begin(), internal.end(), value);
        writePointer = 0;
    }

    /** Returns a pointer to the buffer data, with a given starting position */
    [[nodiscard]] const T* data (int start = 0) const noexcept
    {
#if JUCE_MODULE_AVAILABLE_juce_core
        // need to give the buffer some size before trying to read!
        jassert (size() > 0);
#endif

        start = negativeAwareModulo (start, size());
        return internal.data() + start;
    }

    /** Pushes a new block of data into the buffer */
    void push (const T* data, int numElements)
    {
        const auto currentSize = size();

#if JUCE_MODULE_AVAILABLE_juce_core
        // need to give the buffer some size before trying to push!
        jassert (currentSize > 0);
#endif

        auto* buffer1 = internal.data();
        auto* buffer2 = internal.data() + currentSize;

        if (writePointer + numElements <= size())
        {
            copy_internal (buffer1 + writePointer, data, numElements);
            copy_internal (buffer2 + writePointer, data, numElements);
        }
        else
        {
            const auto samplesTillEnd = currentSize - writePointer;
            copy_internal (buffer1 + writePointer, data, samplesTillEnd);
            copy_internal (buffer2 + writePointer, data, samplesTillEnd);

            const auto leftoverSamples = numElements - samplesTillEnd;
            copy_internal (buffer1, data + samplesTillEnd, leftoverSamples);
            copy_internal (buffer2, data + samplesTillEnd, leftoverSamples);
        }

        writePointer = (writePointer + numElements) % currentSize;
    }

private:
    template <typename C = T>
    std::enable_if_t<std::is_floating_point_v<C>, void>
        copy_internal (T* dest, const T* src, int N)
    {
#if JUCE_MODULE_AVAILABLE_juce_audio_basics
        juce::FloatVectorOperations::copy (dest, src, N);
#else
        std::copy (src, src + N, dest);
#endif
    }

    template <typename C = T>
    std::enable_if_t<! std::is_floating_point_v<C>, void>
        copy_internal (T* dest, const T* src, int N)
    {
        std::copy (src, src + N, dest);
    }

    template <typename IntegerType>
    IntegerType negativeAwareModulo (IntegerType dividend, const IntegerType divisor) const noexcept
    {
        dividend %= divisor;
        return (dividend < 0) ? (dividend + divisor) : dividend;
    }

    std::vector<T> internal;
    int writePointer = 0;
};
} // namespace chowdsp
