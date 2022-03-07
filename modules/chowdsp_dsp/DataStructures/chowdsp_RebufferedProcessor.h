#pragma once

namespace chowdsp
{
/**
 * An abstract class that can be used for implementing
 * audio processing with a fixed buffer size.
 */
template <typename FloatType>
class RebufferedProcessor
{
public:
    RebufferedProcessor() = default;

    /** Initialises the processor. */
    void prepare (const juce::dsp::ProcessSpec& spec);

    /** Resets the processor state */
    void reset();

    /** Processes a block of audio data */
    void processBlock (juce::AudioBuffer<FloatType>& buffer) noexcept;

    /** Returns the latency (samples) introduced by this processor */
    [[nodiscard]] virtual int getLatencySamples() const noexcept { return rebufferSize; }

protected:
    /**
     * Child classes should implement this method to prepare their processing.
     *
     * @return The size of the fixed-size blocks required for the rebuffered processing
     */
    virtual int prepareRebuffering (const juce::dsp::ProcessSpec& spec) = 0;

    /** Child classes should implement this method to perform the rebuffered processing */
    virtual void processRebufferedBlock (juce::AudioBuffer<FloatType>& buffer) = 0;

private:
    void processInternal (juce::AudioBuffer<FloatType>& buffer);
    void pullOutputSignal (juce::AudioBuffer<FloatType>& buffer, int startSample, int samplesToRead) const;
    void pushInputSignal (const juce::AudioBuffer<FloatType>& buffer, int startSample, int samplesToWrite);

    int rebufferSize = 0;
    int numChannelsAllocated = 0;

    juce::AudioBuffer<FloatType> reBuffers[2];
    int bufferCount = 0;
    int writeBufferIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RebufferedProcessor)
};
} // namespace chowdsp
