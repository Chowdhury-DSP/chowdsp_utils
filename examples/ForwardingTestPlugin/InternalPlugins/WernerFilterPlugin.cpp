#include "WernerFilterPlugin.h"

WernerFilterPlugin::WernerFilterPlugin()
{
    morphParam.setParameterHandle (state.params.morphParam.get());
}

void WernerFilterPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const auto spec = juce::dsp::ProcessSpec { sampleRate, (juce::uint32) samplesPerBlock, (juce::uint32) getMainBusNumInputChannels() };
    filter.prepare (spec);

    morphParam.prepare (sampleRate, samplesPerBlock);
    morphParam.setRampLength (0.05);
}

void WernerFilterPlugin::processAudioBlock (juce::AudioBuffer<float>& buffer)
{
    filter.calcCoeffs (*state.params.freqParam, *state.params.dampingParam, *state.params.resonanceParam);
    morphParam.process (buffer.getNumSamples());

    using Type = chowdsp::WernerFilterType;
    const auto mode = state.params.modeParam->getIndex();
    if (mode == 0)
        filter.processBlock<Type::Lowpass2> (buffer);
    else if (mode == 1)
        filter.processBlock<Type::Bandpass2> (buffer);
    else if (mode == 2)
        filter.processBlock<Type::Highpass2> (buffer);
    else if (mode == 3)
    {
        if (morphParam.isSmoothing())
            filter.processBlock<Type::MultiMode> (buffer, morphParam.getSmoothedBuffer());
        else
            filter.processBlock<Type::MultiMode> (buffer, morphParam.getCurrentValue());
    }
    else if (mode == 4)
        filter.processBlock<Type::Lowpass4> (buffer);
}
