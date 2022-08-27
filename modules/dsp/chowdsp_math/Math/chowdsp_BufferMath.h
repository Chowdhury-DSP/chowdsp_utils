#pragma once

namespace chowdsp
{
/** Math operations for working with audio buffers */
namespace BufferMath
{
    /** Computes the absolute magnitude of the buffer. */
    template <typename BufferType>
    static auto getMagnitude (const BufferType& buffer, int startSample = 0, int numSamples = -1, int channel = -1) noexcept;

    /** Computes the RMS level for a single channel of the buffer. */
    template <typename BufferType>
    static auto getRMSLevel (const BufferType& buffer, int channel, int startSample = 0, int numSamples = -1) noexcept;

    /** Copies data from one buffer into another. */
    template <typename BufferType1, typename BufferType2 = BufferType1>
    static void copyBufferData (const BufferType1& bufferSrc, BufferType2& bufferDest, int srcStartSample = 0, int destStartSample = 0, int numSamples = -1, int startChannel = 0, int numChannels = -1) noexcept;

    /** Copies channels from one buffer into another. */
    template <typename BufferType1, typename BufferType2 = BufferType1>
    static void copyBufferChannels (const BufferType1& bufferSrc, BufferType2& bufferDest, int srcChannel, int destChannel) noexcept;

    /** Adds data from one buffer into another. */
    template <typename BufferType1, typename BufferType2 = BufferType1>
    static void addBufferData (const BufferType1& bufferSrc, BufferType2& bufferDest, int srcStartSample = 0, int destStartSample = 0, int numSamples = -1, int startChannel = 0, int numChannels = -1) noexcept;

    /** Adds channels from one buffer into another. */
    template <typename BufferType1, typename BufferType2 = BufferType1>
    static void addBufferChannels (const BufferType1& bufferSrc, BufferType2& bufferDest, int srcChannel, int destChannel) noexcept;

    /** Applies a linear gain to a buffer. */
    template <typename BufferType, typename FloatType = typename BufferType::Type>
    static void applyGain (BufferType& buffer, FloatType gain);

    /** Applies a linear gain to a buffer. */
    template <typename BufferType, typename SmoothedValueType>
    static void applyGainSmoothed (BufferType& buffer, SmoothedValueType& gain);

    /** Applies a linear gain to a buffer. */
    template <typename BufferType, typename SmoothedBufferType>
    static void applyGainSmoothedBuffer (BufferType& buffer, SmoothedBufferType& gain);
} // namespace BufferMath
} // namespace chowdsp

#include "chowdsp_BufferMath.cpp"
