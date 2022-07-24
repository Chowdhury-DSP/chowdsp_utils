#pragma once

namespace chowdsp
{
/**
 * A simple gain processor that fades smoothly
 * across buffers.
 * 
 * @deprecated prefer juce::dsp::Gain<T>
 */
class [[deprecated ("Prefer juce::dsp::Gain or chowdsp::Gain")]] GainProcessor
{
public:
    GainProcessor() = default;

    /** Resets the state of the processor */
    void reset()
    {
        oldGain = curGain;
    }

    /** Processes a buffer of samples */
    void processBlock (juce::AudioBuffer<float> & buffer)
    {
        if (curGain != oldGain)
        {
            buffer.applyGainRamp (0, buffer.getNumSamples(), oldGain, curGain);
            oldGain = curGain;
            return;
        }

        buffer.applyGain (curGain);
    }

    /** Sets the desired linear gain */
    void setGain (float gain)
    {
        if (gain == curGain)
            return;

        oldGain = curGain;
        curGain = gain;
    }

    [[nodiscard]] float getGain() const noexcept { return curGain; }

private:
    float curGain = 1.0f;
    float oldGain = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainProcessor)
};

} // namespace chowdsp
