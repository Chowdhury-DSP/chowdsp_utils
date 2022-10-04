#include "WernerFilterPlugin.h"

namespace
{
const juce::String freqTag = "freq";
const juce::String resonanceTag = "resonance";
const juce::String dampingTag = "damping";
const juce::String morphTag = "morph";
const juce::String modeTag = "mode";
} // namespace

WernerFilterPlugin::WernerFilterPlugin()
{
    using namespace chowdsp::ParamUtils;
    loadParameterPointer (freqParam, vts, freqTag);
    loadParameterPointer (resonanceParam, vts, resonanceTag);
    loadParameterPointer (dampingParam, vts, dampingTag);
    loadParameterPointer (modeParam, vts, modeTag);

    morphParam.setParameterHandle (getParameterPointer<chowdsp::FloatParameter*> (vts, morphTag));
}

void WernerFilterPlugin::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;
    createFreqParameter (params, { freqTag, 100 }, "Frequency", 20.0f, 20000.0f, 2000.0f, 1000.0f);
    createPercentParameter (params, { resonanceTag, 100 }, "Resonance", 0.5f);
    createPercentParameter (params, { dampingTag, 100 }, "Damping", 0.5f);
    createPercentParameter (params, { morphTag, 100 }, "Morph", 0.0f);
    emplace_param<chowdsp::ChoiceParameter> (params, juce::ParameterID { modeTag, 100 }, "Mode", juce::StringArray ("LPF2", "BPF2", "HPF2", "Multi-Mode", "LPF4"), 3);
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
    filter.calcCoeffs (*freqParam, *dampingParam, *resonanceParam);
    morphParam.process (buffer.getNumSamples());

    using Type = chowdsp::WernerFilterType;
    const auto mode = modeParam->getIndex();
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
