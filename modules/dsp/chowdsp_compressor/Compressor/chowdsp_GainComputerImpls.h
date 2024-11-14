#pragma once

namespace chowdsp::compressor
{
/** Internal gain computer parameters */
template <typename T>
struct GainComputerParams
{
    const SmoothedBufferValue<T, juce::ValueSmoothingTypes::Multiplicative>& threshSmooth; // linear gain
    const SmoothedBufferValue<T, juce::ValueSmoothingTypes::Multiplicative>& ratioSmooth;
    T kneeDB;
    T kneeLower; // linear gain
    T kneeUpper; // linear gain
    T makeupGainTarget = (T) 1; // linear gain
};

/** Gain computer for a feed-forward compressor */
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

        const auto threshSmoothData = params.threshSmooth.getSmoothedBuffer();
        const auto ratioSmoothData = params.ratioSmooth.getSmoothedBuffer();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto xData = levelBuffer.getReadSpan (ch);
            auto yData = gainBuffer.getWriteSpan (ch);
            for (size_t n = 0; n < (size_t) numSamples; ++n)
            {
                const auto curThresh = threshSmoothData[n];
                const auto curRatio = ratioSmoothData[n];

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
        }
    }

    void applyAutoMakeup (const BufferView<T>& buffer, const GainComputerParams<T>& params) noexcept
    {
        if (params.threshSmooth.isSmoothing() || params.ratioSmooth.isSmoothing())
        {
            const auto threshSmoothData = params.threshSmooth.getSmoothedBuffer();
            const auto ratioSmoothData = params.ratioSmooth.getSmoothedBuffer();
            for (auto [ch, bufferData] : buffer_iters::channels (buffer))
            {
                for (auto [n, sample] : enumerate (bufferData))
                {
                    const auto curThresh = threshSmoothData[n];
                    const auto curRatio = ratioSmoothData[n];
                    sample *= std::pow ((T) params.makeupGainTarget / curThresh, (T) 1 - ((T) 1 / curRatio));
                }
            }
        }
        else
        {
            const auto curThresh = params.threshSmooth.getCurrentValue();
            const auto curRatio = params.ratioSmooth.getCurrentValue();
            BufferMath::applyGain (buffer, std::pow ((T) params.makeupGainTarget / curThresh, (T) 1 - ((T) 1 / curRatio)));
        }
    }

    T aFF;
};

/** Gain computer for a feed-back compressor */
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

        const auto threshSmoothData = params.threshSmooth.getSmoothedBuffer();
        const auto ratioSmoothData = params.ratioSmooth.getSmoothedBuffer();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto xData = levelBuffer.getReadSpan (ch);
            auto yData = gainBuffer.getWriteSpan (ch);
            for (size_t n = 0; n < (size_t) numSamples; ++n)
            {
                const auto curThresh = threshSmoothData[n];
                const auto curRatio = ratioSmoothData[n];

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
        }
    }

    void applyAutoMakeup (const BufferView<T>& buffer, const GainComputerParams<T>& params) noexcept
    {
        if (params.threshSmooth.isSmoothing() || params.ratioSmooth.isSmoothing())
        {
            const auto threshSmoothData = params.threshSmooth.getSmoothedBuffer();
            const auto ratioSmoothData = params.ratioSmooth.getSmoothedBuffer();
            for (auto [ch, bufferData] : buffer_iters::channels (buffer))
            {
                for (auto [n, sample] : enumerate (bufferData))
                {
                    const auto curThresh = threshSmoothData[n];
                    const auto curRatio = ratioSmoothData[n];
                    sample *= std::pow ((T) params.makeupGainTarget / curThresh, curRatio - (T) 1);
                }
            }
        }
        else
        {
            const auto curThresh = params.threshSmooth.getCurrentValue();
            const auto curRatio = params.ratioSmooth.getCurrentValue();
            BufferMath::applyGain (buffer, std::pow ((T) params.makeupGainTarget / curThresh, curRatio - (T) 1));
        }
    }

    T aFB;
};
} // namespace chowdsp::compressor
