#include "chowdsp_RebufferedProcessor.h"

namespace chowdsp
{
template <typename FloatType>
void RebufferedProcessor<FloatType>::prepare (const juce::dsp::ProcessSpec& spec)
{
    rebufferSize = prepareRebuffering (spec);

    numChannelsAllocated = (int) spec.numChannels;

    for (auto& buffer : reBuffers)
        buffer.setMaxSize (numChannelsAllocated, rebufferSize);

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
void RebufferedProcessor<FloatType>::processBlock (const BufferView<FloatType>& buffer) noexcept
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    jassert (numChannels <= numChannelsAllocated); // too many input channels!

    // Just in case we're prepared for 2 channels, but the caller only wants 1 channel
    for (auto& b : reBuffers)
        b.setCurrentSize (numChannels, rebufferSize);

    // buffer size is small enough to process all at once
    if (numSamples <= rebufferSize)
        return processInternal (buffer);

    // buffer needs to be processed in smaller parts
    BufferView<FloatType> shortBuffer (buffer.getArrayOfWritePointers(), numChannels, rebufferSize, 0);
    processInternal (shortBuffer);

    BufferView<FloatType> leftoverBuffer (buffer.getArrayOfWritePointers(),
                                          numChannels,
                                          numSamples - rebufferSize,
                                          rebufferSize);
    processBlock (leftoverBuffer);
}

template <typename FloatType>
void RebufferedProcessor<FloatType>::processInternal (const BufferView<FloatType>& buffer)
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
        else
        {
            nextBufferProgress (static_cast<double> (bufferCount) / static_cast<double> (rebufferSize));
        }
    }
}

template <typename FloatType>
void RebufferedProcessor<FloatType>::pullOutputSignal (const BufferView<FloatType>& buffer, int startSample, int samplesToRead) const
{
    const auto& readBuffer = reBuffers[1 - writeBufferIndex];
    BufferMath::copyBufferData (readBuffer, buffer, bufferCount, startSample, samplesToRead);
}

template <typename FloatType>
void RebufferedProcessor<FloatType>::pushInputSignal (const BufferView<const FloatType>& buffer, int startSample, int samplesToWrite)
{
    auto& writeBuffer = reBuffers[writeBufferIndex];
    BufferMath::copyBufferData (buffer, writeBuffer, startSample, bufferCount, samplesToWrite);
}

//==============================================================================
#if CHOWDSP_ALLOW_TEMPLATE_INSTANTIATIONS
template class RebufferedProcessor<float>;
template class RebufferedProcessor<double>;
#endif
} // namespace chowdsp
