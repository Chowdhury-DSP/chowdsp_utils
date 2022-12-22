#include "ARPFilterPlugin.h"

ARPFilterPlugin::ARPFilterPlugin()
{
    notchParam.setParameterHandle (state.params.notchOffset.get());
}

void ARPFilterPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const auto spec = juce::dsp::ProcessSpec { sampleRate, (uint32_t) samplesPerBlock, (uint32_t) getMainBusNumInputChannels() };
    filter.prepare (spec);

    notchParam.prepare (sampleRate, samplesPerBlock);
    notchParam.setRampLength (0.02);
}

template <chowdsp::ARPFilterType type>
void processFilter (chowdsp::ARPFilter<float>& filter, juce::AudioBuffer<float>& buffer, chowdsp::SmoothedBufferValue<float>& notchMix)
{
    if (notchMix.isSmoothing())
        filter.processBlock<type> (buffer, notchMix.getSmoothedBuffer());
    else
        filter.processBlock<type> (buffer, notchMix.getCurrentValue());
}

void ARPFilterPlugin::processAudioBlock (juce::AudioBuffer<float>& buffer)
{
    filter.setCutoffFrequency (*state.params.freqParam);
    filter.setQValue (*state.params.qParam);
    filter.setLimitMode (state.params.limitModeParam->get());

    notchParam.process (buffer.getNumSamples());

    const auto modeChoice = state.params.modeParam->getIndex();
    if (modeChoice == 0)
        processFilter<chowdsp::ARPFilterType::Lowpass> (filter, buffer, notchParam);
    else if (modeChoice == 1)
        processFilter<chowdsp::ARPFilterType::Bandpass> (filter, buffer, notchParam);
    else if (modeChoice == 2)
        processFilter<chowdsp::ARPFilterType::Highpass> (filter, buffer, notchParam);
    else if (modeChoice == 3)
        processFilter<chowdsp::ARPFilterType::Notch> (filter, buffer, notchParam);
}
