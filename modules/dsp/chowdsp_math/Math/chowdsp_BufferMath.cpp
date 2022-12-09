#include "chowdsp_BufferMath.h"

namespace chowdsp::BufferMath
{
template <typename BufferType>
auto getMagnitude (const BufferType& buffer, int startSample, int numSamples, int channel) noexcept
{
    if (numSamples < 0)
        numSamples = buffer.getNumSamples() - startSample;

    using SampleType = detail::BufferSampleType<BufferType>;
    auto getChannelMagnitude = [&buffer, startSample, numSamples] (int ch)
    {
        const auto* channelData = buffer.getReadPointer (ch);
        if constexpr (std::is_floating_point_v<SampleType>)
        {
            return FloatVectorOperations::findAbsoluteMaximum (channelData + startSample, numSamples);
        }
#if ! CHOWDSP_NO_XSIMD
        else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
        {
            return std::accumulate (channelData + startSample,
                                    channelData + startSample + numSamples,
                                    SampleType {},
                                    [] (const auto& prev, const auto& next)
                                    {
                                        return xsimd::max (prev, xsimd::abs (next));
                                    });
        }
#endif
    };

    if (channel >= 0)
        return getChannelMagnitude (channel);

    CHOWDSP_USING_XSIMD_STD (max);

    SampleType mag {};
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        mag = max (getChannelMagnitude (ch), mag);
    return mag;
}

template <typename BufferType>
auto getRMSLevel (const BufferType& buffer, int channel, int startSample, int numSamples) noexcept
{
    using SampleType = detail::BufferSampleType<BufferType>;

    if (numSamples < 0)
        numSamples = buffer.getNumSamples() - startSample;

    if (numSamples <= 0 || channel < 0 || channel >= buffer.getNumChannels())
        return SampleType (0);

    auto* data = buffer.getReadPointer (channel) + startSample;
    if constexpr (std::is_floating_point_v<SampleType>)
    {
        return FloatVectorOperations::computeRMS (data, numSamples);
    }
#if ! CHOWDSP_NO_XSIMD
    else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
    {
        using NumericType = SampleTypeHelpers::NumericType<SampleType>;

        auto sum = SampleType (0);
        for (int i = 0; i < numSamples; ++i)
        {
            auto sample = data[i];
            sum += sample * sample;
        }

        return xsimd::sqrt (sum / (NumericType) numSamples);
    }
#endif
}

template <typename BufferType1, typename BufferType2>
void copyBufferData (const BufferType1& bufferSrc, BufferType2& bufferDest, int srcStartSample, int destStartSample, int numSamples, int startChannel, int numChannels) noexcept
{
    using SampleType = detail::BufferSampleType<BufferType1>;
    static_assert (std::is_same_v<SampleType, detail::BufferSampleType<BufferType2>>, "Both buffer types must have the same sample type!");

    if (numSamples < 0)
    {
        jassert (bufferSrc.getNumSamples() == bufferDest.getNumSamples());
        numSamples = bufferDest.getNumSamples();
    }

    if (numChannels < 0)
    {
        jassert (bufferSrc.getNumChannels() == bufferDest.getNumChannels());
        numChannels = bufferDest.getNumChannels();
    }

    jassert (srcStartSample + numSamples <= bufferSrc.getNumSamples());
    jassert (destStartSample + numSamples <= bufferDest.getNumSamples());

    for (int ch = startChannel; ch < startChannel + numChannels; ++ch)
    {
        const auto* srcData = bufferSrc.getReadPointer (ch);
        auto* destData = bufferDest.getWritePointer (ch);

        // If you're here, check that you're calling this function correctly,
        // the channel is probably out of bounds.
        jassert (destData != nullptr);
        jassert (srcData != nullptr);

        if constexpr (std::is_floating_point_v<SampleType>)
        {
            juce::FloatVectorOperations::copy (destData + destStartSample, srcData + srcStartSample, numSamples);
        }
        else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
        {
            std::copy (srcData + srcStartSample, srcData + srcStartSample + numSamples, destData + destStartSample);
        }
    }
}

template <typename BufferType1, typename BufferType2>
void copyBufferChannels (const BufferType1& bufferSrc, BufferType2& bufferDest, int srcChannel, int destChannel) noexcept
{
    using SampleType = detail::BufferSampleType<BufferType1>;
    static_assert (std::is_same_v<SampleType, detail::BufferSampleType<BufferType2>>, "Both buffer types must have the same sample type!");

    jassert (bufferSrc.getNumSamples() == bufferDest.getNumSamples());
    const auto numSamples = bufferSrc.getNumSamples();

    const auto* srcData = bufferSrc.getReadPointer (srcChannel);
    auto* destData = bufferDest.getWritePointer (destChannel);

    if constexpr (std::is_floating_point_v<SampleType>)
    {
        juce::FloatVectorOperations::copy (destData, srcData, numSamples);
    }
    else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
    {
        std::copy (srcData, srcData + numSamples, destData);
    }
}

template <typename BufferType1, typename BufferType2>
void addBufferData (const BufferType1& bufferSrc, BufferType2& bufferDest, int srcStartSample, int destStartSample, int numSamples, int startChannel, int numChannels) noexcept
{
    using SampleType = detail::BufferSampleType<BufferType1>;
    static_assert (std::is_same_v<SampleType, detail::BufferSampleType<BufferType2>>, "Both buffer types must have the same sample type!");

    if (numSamples < 0)
    {
        jassert (bufferSrc.getNumSamples() == bufferDest.getNumSamples());
        numSamples = bufferDest.getNumSamples();
    }

    if (numChannels < 0)
    {
        jassert (bufferSrc.getNumChannels() == bufferDest.getNumChannels());
        numChannels = bufferDest.getNumChannels();
    }

    jassert (srcStartSample + numSamples <= bufferSrc.getNumSamples());
    jassert (destStartSample + numSamples <= bufferDest.getNumSamples());

    for (int ch = startChannel; ch < startChannel + numChannels; ++ch)
    {
        const auto* srcData = bufferSrc.getReadPointer (ch);
        auto* destData = bufferDest.getWritePointer (ch);

        if constexpr (std::is_floating_point_v<SampleType>)
        {
            juce::FloatVectorOperations::add (destData + destStartSample, srcData + srcStartSample, numSamples);
        }
        else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
        {
            std::transform (srcData + srcStartSample,
                            srcData + srcStartSample + numSamples,
                            destData + destStartSample,
                            destData + destStartSample,
                            [] (const auto& a, const auto& b)
                            { return a + b; });
        }
    }
}

/** Adds channels from one buffer into another. */
template <typename BufferType1, typename BufferType2>
void addBufferChannels (const BufferType1& bufferSrc, BufferType2& bufferDest, int srcChannel, int destChannel) noexcept
{
    using SampleType = detail::BufferSampleType<BufferType1>;
    static_assert (std::is_same_v<SampleType, detail::BufferSampleType<BufferType2>>, "Both buffer types must have the same sample type!");

    jassert (bufferSrc.getNumSamples() == bufferDest.getNumSamples());
    const auto numSamples = bufferSrc.getNumSamples();

    const auto* srcData = bufferSrc.getReadPointer (srcChannel);
    auto* destData = bufferDest.getWritePointer (destChannel);

    if constexpr (std::is_floating_point_v<SampleType>)
    {
        juce::FloatVectorOperations::add (destData, srcData, numSamples);
    }
    else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
    {
        std::transform (srcData,
                        srcData + numSamples,
                        destData,
                        destData,
                        [] (const auto& a, const auto& b)
                        { return a + b; });
    }
}

template <typename BufferType, typename FloatType>
void applyGain (BufferType& buffer, FloatType gain) noexcept
{
    using SampleType = detail::BufferSampleType<BufferType>;

    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* data = buffer.getWritePointer (ch);

        if constexpr (std::is_floating_point_v<SampleType>)
        {
            juce::FloatVectorOperations::multiply (data, gain, numSamples);
        }
        else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
        {
            std::transform (data, data + numSamples, data, [gain] (const auto& x)
                            { return x * gain; });
        }
    }
}

template <typename BufferType, typename SmoothedValueType>
void applyGainSmoothed (BufferType& buffer, SmoothedValueType& gain) noexcept
{
    if (! gain.isSmoothing())
    {
        applyGain (buffer, gain.getCurrentValue());
        return;
    }

    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    auto data = buffer.getArrayOfWritePointers();
    for (int n = 0; n < numSamples; ++n)
    {
        const auto sampleGain = gain.getNextValue();

        for (int ch = 0; ch < numChannels; ++ch)
            data[ch][n] *= sampleGain;
    }
}

template <typename BufferType, typename SmoothedBufferType>
void applyGainSmoothedBuffer (BufferType& buffer, SmoothedBufferType& gain) noexcept
{
    using SampleType = detail::BufferSampleType<BufferType>;

    if (! gain.isSmoothing())
    {
        applyGain (buffer, gain.getCurrentValue());
        return;
    }

    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    auto audioData = buffer.getArrayOfWritePointers();
    const auto gainData = gain.getSmoothedBuffer();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        if constexpr (std::is_floating_point_v<SampleType>)
            juce::FloatVectorOperations::multiply (audioData[ch], gainData, numSamples);
        else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
            std::transform (audioData[ch], audioData[ch] + numSamples, gainData, audioData[ch], [] (const auto& x, const auto& g)
                            { return x * g; });
    }
}

template <typename BufferType, typename FloatType>
bool sanitizeBuffer (BufferType& buffer, FloatType ceiling) noexcept
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    bool needsClear = false;
    for (int ch = 0; ch < numChannels; ++ch)
    {
        const auto* channelData = buffer.getReadPointer (ch);
        const auto channelMax = FloatVectorOperations::findAbsoluteMaximum (channelData, numSamples);
        const auto channelNaNs = FloatVectorOperations::countNaNs (channelData, numSamples);
        const auto channelInfs = FloatVectorOperations::countInfs (channelData, numSamples);

        if (channelMax >= ceiling || channelNaNs > 0 || channelInfs > 0)
        {
            // This buffer contains invalid values! Clearing...
            jassertfalse;
            needsClear = true;
        }
    }

    if (needsClear)
        buffer.clear();

    return ! needsClear;
}

template <typename BufferType, typename FunctionType>
void applyFunction (BufferType& buffer, FunctionType&& function)
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* data = buffer.getWritePointer (ch);
        for (int n = 0; n < numSamples; ++n)
            data[n] = function (data[n]);
    }
}

#if ! CHOWDSP_NO_XSIMD
template <typename BufferType, typename FunctionType, typename FloatType>
std::enable_if_t<std::is_floating_point_v<FloatType>, void> applyFunctionSIMD (BufferType& buffer, FunctionType&& function)
{
    applyFunctionSIMD (buffer, std::forward<FunctionType> (function), std::forward<FunctionType> (function));
}

template <typename BufferType, typename SIMDFunctionType, typename ScalarFunctionType, typename FloatType>
std::enable_if_t<std::is_floating_point_v<FloatType>, void> applyFunctionSIMD (BufferType& buffer, SIMDFunctionType&& simdFunction, ScalarFunctionType&& scalarFunction)
{
#if CHOWDSP_USING_JUCE
    static_assert (! std::is_same_v<BufferType, juce::AudioBuffer<FloatType>>,
                   "applyFunctionSIMD expects that the buffers channel data will be aligned"
                   "which cannot be guaranteed with juce::AudioBuffer!");
#endif

    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    static constexpr auto vecSize = (int) xsimd::batch<FloatType>::size;
    auto numVecOps = numSamples / vecSize;
    const auto leftoverValues = numSamples % vecSize;

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* data = buffer.getWritePointer (ch);

        while (--numVecOps >= 0)
        {
            xsimd::store_aligned (data, simdFunction (xsimd::load_aligned (data)));
            data += vecSize;
        }

        for (int n = 0; n < leftoverValues; ++n)
            data[n] = scalarFunction (data[n]);
    }
}
#endif // ! CHOWDSP_NO_XSIMD
} // namespace chowdsp::BufferMath
