#pragma once

namespace chowdsp::compressor
{
/** A multi-channel compressor with monophonic gain reduction. */
template <typename SampleType, typename LevelDetector, typename GainComputer, typename GainReductionMeterTask = NullType>
class MonoCompressor
{
public:
    MonoCompressor() = default;

    struct Params
    {
        float attackMs = 10.0f;
        float releaseMs = 100.0f;
        float thresholdDB = 0.0f;
        float ratio = 1.0f;
        float kneeDB = 6.0f;
        bool autoMakeup = false;
        float autoMakeupTargetDB = 0.0f;
    } params;

    /** Prepares the compressor to an audio stream */
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        levelDetector.prepare (spec.sampleRate, (int) spec.maximumBlockSize);
        gainComputer.prepare (spec.sampleRate, (int) spec.maximumBlockSize);

        levelDetectBuffer.setMaxSize (1, (int) spec.maximumBlockSize);
        gainComputerBuffer.setMaxSize (1, (int) spec.maximumBlockSize);

        if constexpr (! std::is_same_v<GainReductionMeterTask, NullType>)
        {
            // The gain reduction meter always needs to be prepared for 2 channels,
            // one for the input buffer, and one for the output buffer.
            gainReductionMeterTask.prepare (spec.sampleRate, (int) spec.maximumBlockSize, 2);
        }
    }

    /** Processes a block of audio */
    void processBlock (const BufferView<SampleType>& mainBuffer, const BufferView<const SampleType>& keyInputBuffer) noexcept
    {
        const auto numChannels = mainBuffer.getNumChannels();
        const auto numSamples = mainBuffer.getNumSamples();
        jassert (numSamples == keyInputBuffer.getNumSamples());

        levelDetector.setAttackMs (params.attackMs);
        levelDetector.setReleaseMs (params.releaseMs);
        levelDetector.setThresholdDB (params.thresholdDB);

        gainComputer.setThreshold (params.thresholdDB);
        gainComputer.setKnee (params.kneeDB);
        gainComputer.setRatio (params.ratio);

        levelDetectBuffer.setCurrentSize (1, numSamples);
        BufferMath::sumToMono (keyInputBuffer, levelDetectBuffer);
        levelDetector.processBlock (levelDetectBuffer);

        gainComputerBuffer.setCurrentSize (1, numSamples);
        gainComputer.processBlock (levelDetectBuffer, gainComputerBuffer);

        if constexpr (! std::is_same_v<GainReductionMeterTask, NullType>)
            gainReductionMeterTask.pushBufferData (mainBuffer, true);

        for (int ch = 0; ch < numChannels; ++ch)
            juce::FloatVectorOperations::multiply (mainBuffer.getWritePointer (ch), gainComputerBuffer.getReadPointer (0), numSamples);

        if constexpr (! std::is_same_v<GainReductionMeterTask, NullType>)
            gainReductionMeterTask.pushBufferData (mainBuffer, false);

        if (params.autoMakeup)
            gainComputer.applyAutoMakeup (mainBuffer, params.autoMakeupTargetDB);
    }

    LevelDetector levelDetector;
    GainComputer gainComputer;
    GainReductionMeterTask gainReductionMeterTask;

private:
    Buffer<SampleType> levelDetectBuffer;
    Buffer<SampleType> gainComputerBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MonoCompressor)
};
} // namespace chowdsp::compressor
