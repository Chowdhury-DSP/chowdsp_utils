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
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        rebufferSize = prepareRebuffering (spec);

        const int numChannels = (int) spec.numChannels;

        for (auto& buffer : reBuffers)
            buffer.setSize (numChannels, rebufferSize);

        reset();
    }

    /** Resets the processor state */
    void reset()
    {
        for (auto& buffer : reBuffers)
            buffer.clear();

        writeBufferIndex = 0;
        bufferCount = 0;
    }

    /** Processes a block of audio data */
    void processBlock (juce::AudioBuffer<FloatType>& buffer) noexcept
    {
        const auto numSamples = buffer.getNumSamples();

        // buffer size is small enough to process all at once
        if (numSamples <= rebufferSize)
            return processInternal (buffer);

        // buffer needs to be processed in smaller parts
        juce::AudioBuffer<float> shortBuffer (buffer.getArrayOfWritePointers(), buffer.getNumChannels(), 0, rebufferSize);
        processInternal (shortBuffer);

        juce::AudioBuffer<float> leftoverBuffer (buffer.getArrayOfWritePointers(),
                                                 buffer.getNumChannels(),
                                                 rebufferSize,
                                                 numSamples - rebufferSize);
        processBlock (leftoverBuffer);
    }

    /** Returns the latency (samples) introduced by this processor */
    virtual int getLatencySamples() const noexcept { return rebufferSize; }

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
    void processInternal (juce::AudioBuffer<FloatType>& buffer)
    {
        const auto numSamples = buffer.getNumSamples();
        jassert (numSamples <= rebufferSize);

        for (int sampleCount = 0; sampleCount < numSamples;)
        {
            const auto samplesLeft = numSamples - sampleCount;
            int samplesToReadWrite = juce::jmin (samplesLeft, rebufferSize - bufferCount);

            pushInputSignal (buffer, sampleCount, samplesToReadWrite);
            pullOutputSignal (buffer, sampleCount, samplesToReadWrite);

            sampleCount += samplesToReadWrite;
            bufferCount += samplesToReadWrite;

            if (bufferCount == rebufferSize)
            {
                processRebufferedBlock (reBuffers[writeBufferIndex]);
                bufferCount = 0;
                writeBufferIndex = 1 - writeBufferIndex;
            }
        }
    }

    void pullOutputSignal (juce::AudioBuffer<FloatType>& buffer, int startSample, int samplesToRead) const
    {
        const auto& readBuffer = reBuffers[1 - writeBufferIndex];
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.copyFrom (ch, startSample, readBuffer, ch, bufferCount, samplesToRead);
    }

    void pushInputSignal (const juce::AudioBuffer<FloatType>& buffer, int startSample, int samplesToWrite)
    {
        auto& writeBuffer = reBuffers[writeBufferIndex];
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            writeBuffer.copyFrom (ch, bufferCount, buffer, ch, startSample, samplesToWrite);
    }

    int rebufferSize = 0;

    juce::AudioBuffer<float> reBuffers[2];
    int bufferCount = 0;
    int writeBufferIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RebufferedProcessor)
};
} // namespace chowdsp
