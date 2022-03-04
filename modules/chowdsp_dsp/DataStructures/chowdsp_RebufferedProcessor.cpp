#include "chowdsp_RebufferedProcessor.h"

namespace chowdsp
{
template <typename FloatType>
void RebufferedProcessor<FloatType>::prepare (const juce::dsp::ProcessSpec& spec)
{
    rebufferSize = prepareRebuffering (spec);

    numChannelsAllocated = (int) spec.numChannels;

    for (auto& buffer : reBuffers)
        buffer.setSize (numChannelsAllocated, rebufferSize);

    reset();
}

template <typename FloatType>
void RebufferedProcessor<FloatType>::reset()
{
    for (auto& buffer : reBuffers)
        buffer.clear();

    writeBufferIndex = 0;
    bufferCount = 0;
}

template <typename FloatType>
void RebufferedProcessor<FloatType>::processBlock (juce::AudioBuffer<FloatType>& buffer) noexcept
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    jassert (numChannels <= numChannelsAllocated); // too many input channels!

    // Just in case we're prepared for 2 channels, but the caller only wants 1 channel
    for (auto& b : reBuffers)
        b.setSize (numChannels, rebufferSize, true, false, true);

    // buffer size is small enough to process all at once
    if (numSamples <= rebufferSize)
        return processInternal (buffer);

    // buffer needs to be processed in smaller parts
    juce::AudioBuffer<FloatType> shortBuffer (buffer.getArrayOfWritePointers(), numChannels, 0, rebufferSize);
    processInternal (shortBuffer);

    juce::AudioBuffer<FloatType> leftoverBuffer (buffer.getArrayOfWritePointers(),
                                                 numChannels,
                                                 rebufferSize,
                                                 numSamples - rebufferSize);
    processBlock (leftoverBuffer);
}

template <typename FloatType>
void RebufferedProcessor<FloatType>::processInternal (juce::AudioBuffer<FloatType>& buffer)
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

template <typename FloatType>
void RebufferedProcessor<FloatType>::pullOutputSignal (juce::AudioBuffer<FloatType>& buffer, int startSample, int samplesToRead) const
{
    const auto& readBuffer = reBuffers[1 - writeBufferIndex];
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.copyFrom (ch, startSample, readBuffer, ch, bufferCount, samplesToRead);
}

template <typename FloatType>
void RebufferedProcessor<FloatType>::pushInputSignal (const juce::AudioBuffer<FloatType>& buffer, int startSample, int samplesToWrite)
{
    auto& writeBuffer = reBuffers[writeBufferIndex];
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        writeBuffer.copyFrom (ch, bufferCount, buffer, ch, startSample, samplesToWrite);
}

//==============================================================================
template class RebufferedProcessor<float>;
template class RebufferedProcessor<double>;

} // namespace chowdsp
