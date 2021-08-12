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
void BypassProcessor<SampleType, DelayType>::setDelaySamples (int delaySamples)
{
    if (delaySamples == prevDelay)
        return;
        
    compDelay.setDelay ((SampleType) delaySamples);
    if (delaySamples == 0)
        compDelay.reset();

    prevDelay = delaySamples;
}

template <typename SampleType, typename DelayType>
bool BypassProcessor<SampleType, DelayType>::processBlockIn (juce::AudioBuffer<SampleType>& block, bool onOffParam)
{
    enum class DelayOp
    {
        Pop,
        Push,
        Toss,
    };

    auto doDelayOp = [] (auto& sampleBuffer, auto& delay, DelayOp op)
    {
        if (delay.getDelay() == SampleType (0))
            return;

        for (int ch = 0; ch < sampleBuffer.getNumChannels(); ++ch)
        {
            if (op == DelayOp::Push)
            {
                auto* x = sampleBuffer.getWritePointer (ch);
                for (int n = 0; n < sampleBuffer.getNumSamples(); ++n)
                    delay.pushSample (ch, x[n]);
            }
            else if (op == DelayOp::Pop)
            {
                auto* x = sampleBuffer.getWritePointer (ch);
                for (int n = 0; n < sampleBuffer.getNumSamples(); ++n)
                    x[n] = delay.popSample (ch);
            }
            else if (op == DelayOp::Toss)
            {
                for (int n = 0; n < sampleBuffer.getNumSamples(); ++n)
                    delay.incrementReadPointer (ch);
            }
        }
    };

    doDelayOp (block, compDelay, DelayOp::Push);

    if (onOffParam == false && prevOnOffParam == false)
    {
        doDelayOp (block, compDelay, DelayOp::Pop);
        return false;
    }

    if (onOffParam != prevOnOffParam)
    {
        fadeBuffer.makeCopyOf (block, true);
        doDelayOp (fadeBuffer, compDelay, DelayOp::Pop);
    }
    else
    {
        doDelayOp (fadeBuffer, compDelay, DelayOp::Toss);
    }

    return true;
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

    auto doDelayOp = [] (auto& sampleBuffer, auto& delay, DelayOp op)
    {
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

    if (onOffParam == false && prevOnOffParam == false)
    {
        doDelayOp (block, compDelay, DelayOp::Pop);
        return false;
    }

    if (onOffParam != prevOnOffParam)
    {
        fadeBlock.copyFrom (block);
        doDelayOp (fadeBlock, compDelay, DelayOp::Pop);
    }
    else
    {
        doDelayOp (block, compDelay, DelayOp::Toss);
    }

    return true;
}

template <typename SampleType, typename DelayType>
void BypassProcessor<SampleType, DelayType>::processBlockOut (juce::AudioBuffer<SampleType>& block, bool onOffParam)
{
    if (onOffParam == prevOnOffParam)
        return;

    const auto numChannels = block.getNumChannels();
    const auto numSamples = block.getNumSamples();

    SampleType startGain = onOffParam == false ? static_cast<SampleType> (1) // fade out
                                               : static_cast<SampleType> (0); // fade in
    SampleType endGain = static_cast<SampleType> (1) - startGain;

    block.applyGainRamp (0, numSamples, startGain, endGain);
    for (int ch = 0; ch < numChannels; ++ch)
        block.addFromWithRamp (ch, 0, fadeBuffer.getReadPointer (ch), numSamples, 1.0f - startGain, 1.0f - endGain);

    prevOnOffParam = onOffParam;
}

template <typename SampleType, typename DelayType>
void BypassProcessor<SampleType, DelayType>::processBlockOut (juce::dsp::AudioBlock<float>& block, bool onOffParam)
{
    if (onOffParam == prevOnOffParam)
        return;

    const auto numChannels = block.getNumChannels();
    const auto numSamples = block.getNumSamples();

    SampleType startGain = onOffParam == false ? static_cast<SampleType> (1) // fade out
                                               : static_cast<SampleType> (0); // fade in
    SampleType endGain = static_cast<SampleType> (1) - startGain;

    for (size_t ch = 0; ch < numChannels; ++ch)
    {
        auto* blockPtr = block.getChannelPointer (ch);
        auto* fadePtr = fadeBlock.getChannelPointer (ch);

        SampleType gain = startGain;
        SampleType increment = (endGain - startGain) / (SampleType) numSamples;

        for (size_t n = 0; n < numSamples; ++n)
        {
            blockPtr[n] = blockPtr[n] * gain + fadePtr[n] * (static_cast<SampleType> (1) - gain);
            gain += increment;
        }
    }

    prevOnOffParam = onOffParam;
}

} // namespace chowdsp
