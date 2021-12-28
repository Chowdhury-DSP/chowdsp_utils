#include "chowdsp_BypassProcessor.h"

namespace chowdsp
{
template <typename SampleType, typename DelayType>
void BypassProcessor<SampleType, DelayType>::prepare (int samplesPerBlock, bool onOffParam)
{
    prevOnOffParam = onOffParam;
    fadeBuffer.setSize (2, samplesPerBlock);
    fadeBlock = juce::dsp::AudioBlock<SampleType> (fadeBuffer);

    compDelay.prepare ({ 48000.0, (juce::uint32) samplesPerBlock, 2 }); // sample rate does not matter
}

template <typename SampleType, typename DelayType>
void BypassProcessor<SampleType, DelayType>::setLatencySamples (int delaySamples)
{
    if (delaySamples == prevDelay)
        return;

    compDelay.setDelay ((SampleType) delaySamples);

    if (delaySamples == 0)
        compDelay.reset();

    prevDelay = delaySamples;
}

template <typename SampleType, typename DelayType>
bool BypassProcessor<SampleType, DelayType>::processBlockIn (juce::AudioBuffer<SampleType>& buffer, bool onOffParam)
{
    juce::dsp::AudioBlock<float> block (buffer);
    return processBlockIn (block, onOffParam);
}

template <typename SampleType, typename DelayType>
bool BypassProcessor<SampleType, DelayType>::processBlockIn (const juce::dsp::AudioBlock<SampleType>& block, bool onOffParam)
{
    enum class DelayOp
    {
        Pop,
        Push,
        Toss,
    };

    auto doDelayOp = [] (auto& sampleBuffer, auto& delay, DelayOp op) {
        if (delay.getDelay() == SampleType (0))
            return;

        for (int ch = 0; ch < (int) sampleBuffer.getNumChannels(); ++ch)
        {
            if (op == DelayOp::Push)
            {
                auto* x = sampleBuffer.getChannelPointer ((size_t) ch);
                for (int n = 0; n < (int) sampleBuffer.getNumSamples(); ++n)
                    delay.pushSample (ch, x[n]);
            }
            else if (op == DelayOp::Pop)
            {
                auto* x = sampleBuffer.getChannelPointer ((size_t) ch);
                for (int n = 0; n < (int) sampleBuffer.getNumSamples(); ++n)
                    x[n] = delay.popSample (ch);
            }
            else if (op == DelayOp::Toss)
            {
                for (int n = 0; n < (int) sampleBuffer.getNumSamples(); ++n)
                    delay.incrementReadPointer (ch);
            }
        }
    };

    doDelayOp (block, compDelay, DelayOp::Push);

    if (! onOffParam && ! prevOnOffParam)
    {
        doDelayOp (block, compDelay, DelayOp::Pop);
        return false;
    }

    if (onOffParam != prevOnOffParam)
    {
        fadeBlock.copyFrom (block);
        doDelayOp (fadeBlock, compDelay, DelayOp::Pop);

        if (onOffParam && latencySampleCount < 0)
            latencySampleCount = (int) compDelay.getDelay();
    }
    else
    {
        doDelayOp (block, compDelay, DelayOp::Toss);
    }

    return true;
}

template <typename SampleType, typename DelayType>
void BypassProcessor<SampleType, DelayType>::processBlockOut (juce::AudioBuffer<SampleType>& buffer, bool onOffParam)
{
    juce::dsp::AudioBlock<float> block { buffer };
    processBlockOut (block, onOffParam);
}

template <typename SampleType, typename DelayType>
void BypassProcessor<SampleType, DelayType>::processBlockOut (juce::dsp::AudioBlock<float>& block, bool onOffParam)
{
    auto fadeOutputBuffer = [onOffParam] (auto* blockPtr, const auto* fadePtr, const int startSample, const int numSamples) {
        SampleType startGain = ! onOffParam ? static_cast<SampleType> (1) // fade out
                                            : static_cast<SampleType> (0); // fade in
        SampleType endGain = static_cast<SampleType> (1) - startGain;

        SampleType gain = startGain;
        SampleType increment = (endGain - startGain) / (SampleType) (numSamples - startSample);

        juce::FloatVectorOperations::multiply (blockPtr, gain, startSample);
        juce::FloatVectorOperations::addWithMultiply (blockPtr, fadePtr, static_cast<SampleType> (1) - gain, startSample);

        for (int n = startSample; n < numSamples; ++n)
        {
            blockPtr[n] = blockPtr[n] * gain + fadePtr[n] * (static_cast<SampleType> (1) - gain);
            gain += increment;
        }
    };

    if (onOffParam == prevOnOffParam)
    {
        latencySampleCount = 0;
        return;
    }

    const auto numChannels = block.getNumChannels();
    const auto numSamples = (int) block.getNumSamples();
    const auto startSample = getFadeStartSample (numSamples);

    for (size_t ch = 0; ch < numChannels; ++ch)
    {
        auto* blockPtr = block.getChannelPointer (ch);
        auto* fadePtr = fadeBlock.getChannelPointer (ch);

        fadeOutputBuffer (blockPtr, fadePtr, startSample, numSamples);
    }

    if (startSample < numSamples)
        prevOnOffParam = onOffParam;
}

template <typename SampleType, typename DelayType>
int BypassProcessor<SampleType, DelayType>::getFadeStartSample (const int numSamples)
{
    if (latencySampleCount <= 0)
    {
        latencySampleCount = -1;
        return 0;
    }

    if (latencySampleCount < numSamples / 4) // small offset is okay, just adjust the fade start
    {
        int startSample = latencySampleCount;
        latencySampleCount = -1;
        return startSample;
    }
    else // wait for latency time to catch up
    {
        latencySampleCount = juce::jmax (latencySampleCount - numSamples, 0);
        return numSamples;
    }
}

} // namespace chowdsp
