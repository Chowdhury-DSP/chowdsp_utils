#pragma once

namespace chowdsp
{
/** Re-implementation of juce::dsp::Gain, using chowdp::SmoothedBufferValue. */
template <typename FloatType>
class Gain
{
    using NumericType = SampleTypeHelpers::NumericType<FloatType>;

public:
    Gain() = default;

    /** Applies a new gain as a linear value. */
    void setGainLinear (NumericType newGain) noexcept { gainTargetLinear = newGain; }

    /** Applies a new gain as a decibel value. */
    void setGainDecibels (NumericType newGainDecibels) noexcept { setGainLinear (juce::Decibels::decibelsToGain<NumericType> (newGainDecibels)); }

    /** Returns the current gain as a linear value. */
    NumericType getGainLinear() const noexcept { return gainTargetLinear; }

    /** Returns the current gain in decibels. */
    NumericType getGainDecibels() const noexcept { return juce::Decibels::gainToDecibels<NumericType> (getGainLinear()); }

    /** Sets the length of the ramp used for smoothing gain changes. */
    void setRampDurationSeconds (double newDurationSeconds) noexcept
    {
        if (! juce::approximatelyEqual (rampDurationSeconds, newDurationSeconds))
        {
            rampDurationSeconds = newDurationSeconds;
            gain.setRampLength (rampDurationSeconds);
        }
    }

    /** Returns the ramp duration in seconds. */
    [[nodiscard]] double getRampDurationSeconds() const noexcept { return rampDurationSeconds; }

    /** Returns true if the current value is currently being interpolated. */
    [[nodiscard]] bool isSmoothing() const noexcept
    {
        return gain.isSmoothing() || ! juce::approximatelyEqual (gainTargetLinear, gain.getCurrentValue());
    }

    //==============================================================================
    /** Called before processing starts. */
    void prepare (const juce::dsp::ProcessSpec& spec) noexcept
    {
        sampleRate = spec.sampleRate;
        gain.prepare (spec.sampleRate, (int) spec.maximumBlockSize);
        reset();
    }

    /** Resets the internal state of the gain */
    void reset() noexcept
    {
        gain.reset (gainTargetLinear);
    }

    /** Processes the Buffer. */
    void process (const BufferView<FloatType>& buffer) noexcept
    {
        gain.process (gainTargetLinear, buffer.getNumSamples());
        BufferMath::applyGainSmoothedBuffer (buffer, gain);
    }

private:
    SmoothedBufferValue<NumericType> gain;
    NumericType gainTargetLinear = (NumericType) 0;
    double sampleRate = 0, rampDurationSeconds = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Gain)
};
} // namespace chowdsp
