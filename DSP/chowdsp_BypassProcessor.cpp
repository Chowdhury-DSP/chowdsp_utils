namespace chowdsp
{

template <typename SampleType, typename DelayType>
void BypassProcessor<SampleType, DelayType>::prepare (int samplesPerBlock, bool onOffParam)
{
    prevOnOffParam = onOffParam;
    fadeBuffer.setSize (2, samplesPerBlock);
    fadeBlock = juce::dsp::AudioBlock<SampleType> (fadeBuffer);

    compDelay = std::make_unique<DelayLine<float, DelayType>> (samplesPerBlock);
    compDelay->prepare ({ 48000.0, (juce::uint32) samplesPerBlock, 2 }); // sample rate does not matter
}

template <typename SampleType, typename DelayType>
void BypassProcessor<SampleType, DelayType>::setDelaySamples (SampleType delaySamples)
{
    if (delaySamples == prevDelay)
        return;
        
    compDelay->setDelay (delaySamples);
    if (delaySamples == SampleType (0))
        compDelay->reset();

    prevDelay = delaySamples;
}

template <typename SampleType, typename DelayType>
bool BypassProcessor<SampleType, DelayType>::processBlockIn (juce::AudioBuffer<SampleType>& block, bool onOffParam)
{
    if (compDelay->getDelay() > SampleType (0))
    {
        for (int ch = 0; ch < block.getNumChannels(); ++ch)
        {
            auto* x = block.getReadPointer (ch);
            for (int n = 0; n < block.getNumSamples(); ++n)
                compDelay->pushSample (ch, x[n]);
        }
    }

    if (onOffParam == false && prevOnOffParam == false)
        return false;

    if (compDelay->getDelay() > SampleType (0))
    {
        for (int ch = 0; ch < block.getNumChannels(); ++ch)
        {
            auto* x = block.getWritePointer (ch);
            for (int n = 0; n < block.getNumSamples(); ++n)
                x[n] = compDelay->popSample (ch);
        }
    }

    if (onOffParam != prevOnOffParam)
        fadeBuffer.makeCopyOf (block, true);

    return true;
}

template <typename SampleType, typename DelayType>
bool BypassProcessor<SampleType, DelayType>::processBlockIn (const juce::dsp::AudioBlock<SampleType>& block, bool onOffParam)
{
    if (compDelay->getDelay() > SampleType (0))
    {
        for (int ch = 0; ch < (int) block.getNumChannels(); ++ch)
        {
            auto* x = block.getChannelPointer ((size_t) ch);
            for (int n = 0; n < (int) block.getNumSamples(); ++n)
                compDelay->pushSample (ch, x[n]);
        }
    }

    if (onOffParam == false && prevOnOffParam == false)
        return false;

    if (compDelay->getDelay() > SampleType (0))
    {
        for (int ch = 0; ch < (int) block.getNumChannels(); ++ch)
        {
            auto* x = block.getChannelPointer ((size_t) ch);
            for (int n = 0; n < (int) block.getNumSamples(); ++n)
                x[n] = compDelay->popSample (ch);
        }
    }

    if (onOffParam != prevOnOffParam)
        fadeBlock.copyFrom (block);

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
