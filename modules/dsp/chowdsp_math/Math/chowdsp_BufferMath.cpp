#include "chowdsp_BufferMath.h"

namespace chowdsp::BufferMath
{
    template <typename BufferType>
    auto getMagnitude (const BufferType& buffer, int startSample, int numSamples, int channel) noexcept
    {
        if (numSamples < 0)
            numSamples = buffer.getNumSamples() - startSample;

        using SampleType = typename BufferType::Type;
        auto getChannelMagnitude = [&buffer, startSample, numSamples] (int ch)
        {
            const auto* channelData = buffer.getReadPointer (ch);
            if constexpr (std::is_floating_point_v<SampleType>)
            {
                return FloatVectorOperations::findAbsoluteMaximum (channelData + startSample, numSamples);
            }
            else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
            {
                return std::accumulate (channelData + startSample,
                                        channelData + startSample + numSamples,
                                        SampleType {},
                                        [](const auto& prev, const auto& next)
                                        {
                                            return xsimd::max (prev, xsimd::abs (next));
                                        });
            }
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
        else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
        {
            auto sum = SampleType (0);
            for (int i = 0; i < numSamples; ++i)
            {
                auto sample = data[i];
                sum += sample * sample;
            }

            return xsimd::sqrt (sum / (SampleType) numSamples);
        }
    }

    template <typename BufferType1, typename BufferType2>
    void copyBufferData (const BufferType1& bufferSrc, BufferType2& bufferDest, int srcStartSample, int destStartSample, int numSamples, int startChannel, int numChannels) noexcept
    {
        using SampleType = typename BufferType1::Type;
        static_assert (std::is_same_v<SampleType, typename BufferType2::Type>, "Buffers do not have the same underlying type!");

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
                juce::FloatVectorOperations::copy (destData + destStartSample, srcData + srcStartSample, numSamples);
            }
            else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
            {
                std::copy (srcData + srcStartSample, srcData + srcStartSample + numSamples, destData + destStartSample);
            }
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
                std::transform (data, data + numSamples, data, [gain](const auto& x) { return x * gain; });
            }
        }
    }
} // namespace chowdsp
