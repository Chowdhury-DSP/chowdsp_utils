#pragma once

namespace chowdsp::compressor
{
template <typename T>
struct GainComputerParams
{
    const T* threshSmoothData;
    const T* ratioSmoothData;
    T kneeDB;
    T kneeLower;
    T kneeUpper;
    bool applyAutoMakeup;
};

template <typename T>
struct FeedForwardCompGainComputer
{
    void recalcConstants (T ratio, T kneeDB)
    {
        aFF = ((T) 1 - ((T) 1 / ratio)) / ((T) 2 * kneeDB);
    }

    void process (const BufferView<const T>& levelBuffer, const BufferView<T>& gainBuffer, const GainComputerParams<T>& params) noexcept
    {
        const auto numChannels = gainBuffer.getNumChannels();
        const auto numSamples = gainBuffer.getNumSamples();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto xData = levelBuffer.getReadSpan (ch);
            auto yData = gainBuffer.getWriteSpan (ch);
            for (size_t n = 0; n < (size_t) numSamples; ++n)
            {
                const auto curThresh = params.threshSmoothData[n];
                const auto curRatio = params.ratioSmoothData[n];

                const auto xAbs = std::abs (xData[n]);
                if (xAbs <= params.kneeLower) // below thresh
                {
                    yData[n] = (T) 1;
                }
                else if (xAbs >= params.kneeUpper) // compression range
                {
                    yData[n] = std::pow (xAbs / curThresh, ((T) 1 / curRatio) - (T) 1);
                }
                else
                {
                    // knee range
                    auto gainCorr = juce::Decibels::gainToDecibels (xAbs) - juce::Decibels::gainToDecibels (curThresh) + (T) 0.5 * params.kneeDB;
                    auto gainDB = (T) -1 * aFF * gainCorr * gainCorr;
                    yData[n] = juce::Decibels::decibelsToGain (gainDB);
                }
            }

            if (params.applyAutoMakeup)
            {
                for (size_t n = 0; n < (size_t) numSamples; ++n)
                {
                    const auto curThresh = params.threshSmoothData[n];
                    const auto curRatio = params.ratioSmoothData[n];
                    yData[n] *= std::pow ((T) 1 / curThresh, (T) 1 - ((T) 1 / curRatio));
                }
            }
        }
    }

    T aFF;
};

template <typename T>
struct FeedBackCompGainComputer
{
    void recalcConstants (T ratio, T kneeDB)
    {
        aFB = (ratio - (T) 1) / ((T) 2 * kneeDB);
    }

    void process (const BufferView<const T>& levelBuffer, const BufferView<T>& gainBuffer, const GainComputerParams<T>& params) noexcept
    {
        const auto numChannels = gainBuffer.getNumChannels();
        const auto numSamples = gainBuffer.getNumSamples();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto xData = levelBuffer.getReadSpan (ch);
            auto yData = gainBuffer.getWriteSpan (ch);
            for (size_t n = 0; n < (size_t) numSamples; ++n)
            {
                const auto curThresh = params.threshSmoothData[n];
                const auto curRatio = params.ratioSmoothData[n];

                const auto xAbs = std::abs (xData[n]);
                if (xAbs <= params.kneeLower) // below thresh
                {
                    yData[n] = (T) 1;
                }
                else if (xAbs >= params.kneeUpper) // compression range
                {
                    yData[n] = std::pow (xAbs / curThresh, (T) 1 - curRatio);
                }
                else // knee range
                {
                    auto gainCorr = juce::Decibels::gainToDecibels (xAbs) - juce::Decibels::gainToDecibels (curThresh) + (T) 0.5 * params.kneeDB;
                    auto gainDB = (T) -1 * aFB * gainCorr * gainCorr;
                    yData[n] = juce::Decibels::decibelsToGain (gainDB);
                }
            }

            if (params.applyAutoMakeup)
            {
                for (size_t n = 0; n < (size_t) numSamples; ++n)
                {
                    const auto curThresh = params.threshSmoothData[n];
                    const auto curRatio = params.ratioSmoothData[n];
                    yData[n] *= std::pow ((T) 1 / curThresh, curRatio - (T) 1);
                }
            }
        }
    }

    T aFB;
};
}
