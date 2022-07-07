#pragma once

namespace chowdsp
{
class BufferMath
{
public:
    template <typename BufferType>
    static auto getMagnitude (const BufferType& buffer, int startSample = 0, int numSamples = -1, int channel = -1) noexcept
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

private:
    BufferMath() = delete; // static use only!
};
} // namespace chowdsp
