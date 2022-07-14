#include "WernerFilterPlugin.h"

namespace
{
const juce::String freqTag = "freq";
const juce::String resonanceTag = "resonance";
const juce::String dampingTag = "damping";
const juce::String modeTag = "mode";
} // namespace

WernerFilterPlugin::WernerFilterPlugin()
{
    using namespace chowdsp::ParamUtils;
    loadParameterPointer (freqParam, vts, freqTag);
    loadParameterPointer (resonanceParam, vts, resonanceTag);
    loadParameterPointer (dampingParam, vts, dampingTag);
    loadParameterPointer (modeParam, vts, modeTag);
}

void WernerFilterPlugin::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;
    createFreqParameter (params, freqTag, "Frequency", 20.0f, 20000.0f, 2000.0f, 1000.0f);
    emplace_param<chowdsp::FloatParameter> (params, resonanceTag, "Resonance", createNormalisableRange (0.0f, 0.95f, 0.7071f), 0.7071f, &floatValToString, &stringToFloatVal);
    emplace_param<chowdsp::FloatParameter> (params, dampingTag, "Damping", createNormalisableRange (0.3f, 1.25f, 0.7071f), 0.7071f, &floatValToString, &stringToFloatVal);
    emplace_param<chowdsp::ChoiceParameter> (params, modeTag, "Mode", juce::StringArray ("LPF2", "BPF2", "HPF2", "LPF4"), 3);
}

void WernerFilterPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const auto spec = juce::dsp::ProcessSpec { sampleRate, (juce::uint32) samplesPerBlock, (juce::uint32) getMainBusNumInputChannels() };
    filter.prepare (spec);
}

void WernerFilterPlugin::processAudioBlock (juce::AudioBuffer<float>& buffer)
{
    filter.calcCoeffs (*freqParam, *dampingParam, *resonanceParam);

    using Type = chowdsp::WernerFilterType;
    const auto mode = modeParam->getIndex();
    if (mode == 0)
        filter.processBlock<Type::Lowpass2> (buffer);
    else if (mode == 1)
        filter.processBlock<Type::Bandpass2> (buffer);
    else if (mode == 2)
        filter.processBlock<Type::Highpass2> (buffer);
    else if (mode == 3)
        filter.processBlock<Type::Lowpass4> (buffer);
}
