#pragma once

namespace chowdsp
{
/** Math operations for working with audio buffers */
namespace BufferMath
{
    /** Computes the absolute magnitude of the buffer. */
    template <typename BufferType>
    auto getMagnitude (const BufferType& buffer, int startSample = 0, int numSamples = -1, int channel = -1) noexcept;

    /** Computes the RMS level for a single channel of the buffer. */
    template <typename BufferType>
    auto getRMSLevel (const BufferType& buffer, int channel, int startSample = 0, int numSamples = -1) noexcept;

    /** Copies data from one buffer into another. */
    template <typename BufferType1, typename BufferType2 = BufferType1>
    void copyBufferData (const BufferType1& bufferSrc, BufferType2& bufferDest, int srcStartSample = 0, int destStartSample = 0, int numSamples = -1, int startChannel = 0, int numChannels = -1) noexcept;

    /** Copies channels from one buffer into another. */
    template <typename BufferType1, typename BufferType2 = BufferType1>
    void copyBufferChannels (const BufferType1& bufferSrc, BufferType2& bufferDest, int srcChannel, int destChannel) noexcept;

    /** Adds data from one buffer into another. */
    template <typename BufferType1, typename BufferType2 = BufferType1>
    void addBufferData (const BufferType1& bufferSrc, BufferType2& bufferDest, int srcStartSample = 0, int destStartSample = 0, int numSamples = -1, int startChannel = 0, int numChannels = -1) noexcept;

    /** Adds channels from one buffer into another. */
    template <typename BufferType1, typename BufferType2 = BufferType1>
    void addBufferChannels (const BufferType1& bufferSrc, BufferType2& bufferDest, int srcChannel, int destChannel) noexcept;

    /** Multiplies data from one buffer by another. */
    template <typename BufferType1, typename BufferType2 = BufferType1>
    void multiplyBufferData (const BufferType1& bufferSrc, BufferType2& bufferDest, int srcStartSample = 0, int destStartSample = 0, int numSamples = -1, int startChannel = 0, int numChannels = -1) noexcept;

    /** Applies a linear gain to a buffer. */
    template <typename BufferType, typename FloatType = typename BufferType::Type>
    void applyGain (BufferType& buffer, FloatType gain) noexcept;

    /** Applies a linear gain to a buffer and stores the result in another buffer. */
    template <typename BufferType1, typename BufferType2 = BufferType1, typename FloatType = typename BufferType1::Type>
    void applyGain (const BufferType1& bufferSrc, BufferType2& bufferDest, FloatType gain) noexcept;

    /** Applies a linear gain to a buffer. */
    template <typename BufferType, typename SmoothedValueType>
    void applyGainSmoothed (BufferType& buffer, SmoothedValueType& gain) noexcept;

    /** Applies a linear gain to a buffer and stores the result in another buffer. */
    template <typename BufferType1, typename SmoothedValueType, typename BufferType2 = BufferType1>
    void applyGainSmoothed (const BufferType1& bufferSrc, BufferType2& bufferDest, SmoothedValueType& gain) noexcept;

    /** Applies a linear gain to a buffer. */
    template <typename BufferType, typename SmoothedBufferType>
    void applyGainSmoothedBuffer (BufferType& buffer, SmoothedBufferType& gain) noexcept;

    /** Applies a linear gain to a buffer and stores the result in another buffer. */
    template <typename BufferType1, typename SmoothedBufferType, typename BufferType2 = BufferType1>
    void applyGainSmoothedBuffer (const BufferType1& bufferSrc, BufferType2& bufferDest, SmoothedBufferType& gain) noexcept;

    /**
     * Sums the channels in the source buffer, and stores
     * the result in the first channel of the destination buffer.
     * Other channels in the destination buffer will be unaffected.
     *
     * The caller can pass a custom normalization gain value. If a negative
     * value is passed, a "default" normalization value of 1 / bufferSrc.getNumChannels()
     * will be applied.
     */
    template <typename BufferType1, typename BufferType2, typename FloatType = SampleTypeHelpers::NumericType<BufferSampleType<BufferType1>>>
    void sumToMono (const BufferType1& bufferSrc, BufferType2& bufferDest, FloatType normGain = (FloatType) -1);

    /**
     * If the buffer contains any Infs, NaNs, or values larger than the ceiling,
     * this method will clear the buffer and return false.
     */
    template <typename BufferType, typename FloatType = BufferSampleType<BufferType>>
    bool sanitizeBuffer (BufferType& buffer, FloatType ceiling = (FloatType) 100) noexcept;

    /** Applies a function to every sample in the buffer. */
    template <typename BufferType, typename FunctionType>
    void applyFunction (BufferType& buffer, FunctionType&& function) noexcept;

    /** Applies a function to every sample in the buffer and stores the result in another buffer. */
    template <typename BufferType1, typename BufferType2, typename FunctionType>
    void applyFunction (const BufferType1& bufferSrc, BufferType2& bufferDest, FunctionType&& function) noexcept;

#if ! CHOWDSP_NO_XSIMD
    /** Applies a SIMD/scalar function to every sample in the buffer. */
    template <typename BufferType, typename FunctionType, typename FloatType = BufferSampleType<BufferType>>
    std::enable_if_t<std::is_floating_point_v<FloatType>, void> applyFunctionSIMD (BufferType& buffer, FunctionType&& function) noexcept;

    /** Applies a SIMD/scalar function to every sample in the buffer and stores the result in another buffer. */
    template <typename BufferType1, typename BufferType2, typename FunctionType, typename FloatType = BufferSampleType<BufferType1>>
    std::enable_if_t<std::is_floating_point_v<FloatType>, void>
        applyFunctionSIMD (const BufferType1& bufferSrc, BufferType2& bufferDest, FunctionType&& function) noexcept;

    /** Applies a SIMD or scalar function to every sample in the buffer. */
    template <typename BufferType, typename SIMDFunctionType, typename ScalarFunctionType, typename FloatType = BufferSampleType<BufferType>>
    std::enable_if_t<std::is_floating_point_v<FloatType>, void>
        applyFunctionSIMD (BufferType& buffer, SIMDFunctionType&& simdFunction, ScalarFunctionType&& scalarFunction) noexcept;

    /** Applies a SIMD or scalar function to every sample in the buffer and stores the result in another buffer. */
    template <typename BufferType1, typename BufferType2, typename SIMDFunctionType, typename ScalarFunctionType, typename FloatType = BufferSampleType<BufferType1>>
    std::enable_if_t<std::is_floating_point_v<FloatType>, void>
        applyFunctionSIMD (const BufferType1& bufferSrc, BufferType2& bufferDest, SIMDFunctionType&& simdFunction, ScalarFunctionType&& scalarFunction) noexcept;
#endif
} // namespace BufferMath
} // namespace chowdsp

#include "chowdsp_BufferMath.cpp"
