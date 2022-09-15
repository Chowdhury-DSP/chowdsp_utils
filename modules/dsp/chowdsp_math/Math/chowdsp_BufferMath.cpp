#include "chowdsp_BufferMath.h"

namespace chowdsp::BufferMath
{
template <typename BufferType>
auto getMagnitude (const BufferType& buffer, int startSample, int numSamples, int channel) noexcept
{
    if (numSamples < 0)
        numSamples = buffer.getNumSamples() - startSample;

    using SampleType = typename BufferType::Type;
    auto getChannelMagnitude = [&buffer, startSample, numSamples] (int ch) {
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
                                    [] (const auto& prev, const auto& next) {
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
    using SampleType = typename BufferType::Type;

    if (numSamples < 0)
        numSamples = buffer.getNumSamples() - startSample;

    if (numSamples <= 0 || channel < 0 || channel >= buffer.getNumChannels())
        return SampleType (0);

    auto* data = buffer.getReadPointer (channel) + startSample;
    if constexpr (std::is_floating_point_v<SampleType>)
    {
        return chowdsp::FloatVectorOperations::computeRMS (data, numSamples);
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
    using SampleType = typename BufferType1::Type;

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
        jassert(destData != nullptr);
        jassert(srcData != nullptr);

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
    using SampleType = typename BufferType1::Type;

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
    using SampleType = typename BufferType1::Type;

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
                            [] (const auto& a, const auto& b) { return a + b; });
        }
    }
}

/** Adds channels from one buffer into another. */
template <typename BufferType1, typename BufferType2>
void addBufferChannels (const BufferType1& bufferSrc, BufferType2& bufferDest, int srcChannel, int destChannel) noexcept
{
    using SampleType = typename BufferType1::Type;

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
                        [] (const auto& a, const auto& b) { return a + b; });
    }
}

template <typename BufferType, typename FloatType>
void applyGain (BufferType& buffer, FloatType gain)
{
    using SampleType = typename BufferType::Type;

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
            std::transform (data, data + numSamples, data, [gain] (const auto& x) { return x * gain; });
        }
    }
}

template <typename BufferType, typename SmoothedValueType>
void applyGainSmoothed (BufferType& buffer, SmoothedValueType& gain)
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
void applyGainSmoothedBuffer (BufferType& buffer, SmoothedBufferType& gain)
{
    using SampleType = typename BufferType::Type;

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
            std::transform (audioData[ch], audioData[ch] + numSamples, gainData, audioData[ch], [] (const auto& x, const auto& g) { return x * g; });
    }
}
} // namespace chowdsp::BufferMath
