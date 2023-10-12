#pragma once

namespace chowdsp
{
/**
 * A simple limiter designed to be used to protect against overshoot.
 *
 * A typical use-case might be after a clipper or limiter being applied
 * in an up-sampled process. Since the down-sampling process might introduce
 * overshoot not present in the up-sampled signal, this limiter can be used
 * to maintain the "ceiling" provided by the clipper/limiter after down-sampling.
 *
 * Note that this limiter is designed to provide something like 0.1 dB of gain
 * reduction. Using it as you would a normal limiter (maybe 1-5 dB of gain reduction)
 * probably won't work very well.
 */
template <typename SampleType>
class OvershootLimiter
{
public:
    /** Constructs the limiter with a given lookahead characteristic */
    explicit OvershootLimiter (int lookaheadInSamples = 32)
        : lookaheadSamples (lookaheadInSamples)
    {
    }

    /** Prepares the limiter */
    void prepare (const juce::dsp::ProcessSpec spec)
    {
        makeupDelay.prepare (spec);
        makeupDelay.setDelay ((SampleType) lookaheadSamples);
        makeupGain.reset (lookaheadSamples);

        reset();
    }

    /** Prepares the limiter state */
    void reset()
    {
        makeupDelay.reset();
        makeupGain.setCurrentAndTargetValue ((SampleType) 1);
        lastBlockMakeupGain = (SampleType) 1;
    }

    /** Sets the limiter's "ceiling" (default value is 1) */
    void setCeiling (SampleType newCeiling)
    {
        ceiling = newCeiling;
    }

    /** Returns the latency added by the limiter due to its "lookahead" characteristic */
    [[nodiscard]] int getLatencySamples() const noexcept
    {
        return lookaheadSamples;
    }

    /** Processes a buffer of audio */
    void processBlock (const BufferView<SampleType>& buffer) noexcept
    {
        auto signalAbsMax = (SampleType) ceiling;
        for (auto [ch, data] : buffer_iters::channels (buffer))
        {
            signalAbsMax = juce::jmax (signalAbsMax,
                                       FloatVectorOperations::findAbsoluteMaximum (data.data(), (int) data.size()));
        }

        const auto thisBlockMakeupGain = ceiling / signalAbsMax;
        jassert (thisBlockMakeupGain <= (SampleType) 1);
        makeupGain.setTargetValue (juce::jmin (thisBlockMakeupGain, lastBlockMakeupGain));
        lastBlockMakeupGain = thisBlockMakeupGain;

        BufferMath::applyGainSmoothed (buffer, makeupGain);
    }

private:
    const int lookaheadSamples = 32;
    DelayLine<SampleType, DelayLineInterpolationTypes::None> makeupDelay { lookaheadSamples };
    juce::SmoothedValue<SampleType, juce::ValueSmoothingTypes::Multiplicative> makeupGain { (SampleType) 1 };
    SampleType lastBlockMakeupGain { (SampleType) 1 };

    SampleType ceiling = (SampleType) 1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OvershootLimiter)
};
} // namespace chowdsp
