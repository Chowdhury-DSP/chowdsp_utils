#pragma once

namespace chowdsp
{
// @TODO: write tests for all of these:

/** Math operations for working with audio buffers */
namespace BufferMath
{
    template <typename BufferType>
    static auto getMagnitude (const BufferType& buffer, int startSample = 0, int numSamples = -1, int channel = -1) noexcept;

    template <typename BufferType>
    static auto getRMSLevel (const BufferType& buffer, int channel, int startSample = 0, int numSamples = -1) noexcept;

    template <typename BufferType1, typename BufferType2 = BufferType1>
    static void copyBufferData (const BufferType1& bufferSrc, BufferType2& bufferDest, int srcStartSample = 0, int destStartSample = 0, int numSamples = -1, int startChannel = 0, int numChannels = -1) noexcept;

    template <typename BufferType, typename FloatType = typename BufferType::Type>
    static void applyGain (BufferType& buffer, FloatType gain);
} // namespace BufferMath
} // namespace chowdsp

#include "chowdsp_BufferMath.cpp"
