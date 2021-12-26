#pragma once

#include <algorithm>
#include <type_traits>
#include <vector>

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
    int size() const noexcept { return (int) internal.size() / 2; }

    /** Returns the current position of the buffer's write pointer */
    int getWritePointer() const noexcept { return writePointer; }

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
        if constexpr (std::is_floating_point<T>::value)
            juce::FloatVectorOperations::fill (internal.data(), value, size() * 2);
        else
            std::fill (internal.begin(), internal.end(), value);

        writePointer = 0;
    }

    /** Returns a pointer to the buffer data, with a given starting position */
    const T* data (int start = 0) const noexcept
    {
        // need to give the buffer some size before trying to read!
        jassert (size() > 0);

        start = juce::negativeAwareModulo (start, size());
        return internal.data() + start;
    }

    /** Pushes a new block of data into the buffer */
    void push (const T* data, int numElements)
    {
        const auto currentSize = size();

        // need to give the buffer some size before trying to push!
        jassert (currentSize > 0);

        auto* buffer1 = internal.data();
        auto* buffer2 = internal.data() + currentSize;

        // for floating point types we can use juce::FloatVectorOperations for vectorized copy!
        if constexpr (std::is_floating_point<T>::value)
        {
            if (writePointer + numElements <= size())
            {
                juce::FloatVectorOperations::copy (buffer1 + writePointer, data, numElements);
                juce::FloatVectorOperations::copy (buffer2 + writePointer, data, numElements);
            }
            else
            {
                const auto samplesTillEnd = currentSize - writePointer;
                juce::FloatVectorOperations::copy (buffer1 + writePointer, data, samplesTillEnd);
                juce::FloatVectorOperations::copy (buffer2 + writePointer, data, samplesTillEnd);

                const auto leftoverSamples = numElements - samplesTillEnd;
                juce::FloatVectorOperations::copy (buffer1, data + samplesTillEnd, leftoverSamples);
                juce::FloatVectorOperations::copy (buffer2, data + samplesTillEnd, leftoverSamples);
            }

            writePointer = (writePointer + numElements) % currentSize;
        }
        else
        {
            if (writePointer + numElements <= size())
            {
                std::copy (data, data + numElements, buffer1 + writePointer);
                std::copy (data, data + numElements, buffer2 + writePointer);
            }
            else
            {
                const auto samplesTillEnd = currentSize - writePointer;
                std::copy (data, data + samplesTillEnd, buffer1 + writePointer);
                std::copy (data, data + samplesTillEnd, buffer2 + writePointer);

                std::copy (data + samplesTillEnd, data + numElements, buffer1);
                std::copy (data + samplesTillEnd, data + numElements, buffer2);
            }

            writePointer = (writePointer + numElements) % currentSize;
        }
    }

private:
    std::vector<T> internal;
    int writePointer = 0;
};
} // namespace chowdsp
