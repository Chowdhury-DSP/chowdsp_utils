#include "ARPFilterPlugin.h"

namespace
{
const juce::String freqTag = "freq";
const juce::String qTag = "q_val";
const juce::String limitModeTag = "limit_mode";
const juce::String notchTag = "notch";
const juce::String modeTag = "mode";
} // namespace

ARPFilterPlugin::ARPFilterPlugin()
{
    using namespace chowdsp::ParamUtils;
    loadParameterPointer (freqParam, vts, freqTag);
    loadParameterPointer (qParam, vts, qTag);
    loadParameterPointer (limitModeParam, vts, limitModeTag);
    loadParameterPointer (modeParam, vts, modeTag);

    notchParam.setParameterHandle (getParameterPointer<chowdsp::FloatParameter*> (vts, notchTag));
}

void ARPFilterPlugin::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;
    createFreqParameter (params, { freqTag, 100 }, "Frequency", 20.0f, 20000.0f, 2000.0f, 1000.0f);
    emplace_param<chowdsp::FloatParameter> (params, juce::ParameterID { qTag, 100 }, "Q", createNormalisableRange (0.5f, 2.5f, 0.7071f), 0.7071f, &floatValToString, &stringToFloatVal);
    emplace_param<chowdsp::BoolParameter> (params, juce::ParameterID { limitModeTag, 100 }, "Limit Mode", false);
    createBipolarPercentParameter (params, { notchTag, 100 }, "Notch Offset", 0.0f);
    emplace_param<chowdsp::ChoiceParameter> (params, juce::ParameterID { modeTag, 100 }, "Mode", juce::StringArray ("LPF2", "BPF2", "HPF2", "Notch"), 0);
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
    filter.setCutoffFrequency (*freqParam);
    filter.setQValue (*qParam);
    filter.setLimitMode (limitModeParam->get());

    notchParam.process (buffer.getNumSamples());

    const auto modeChoice = modeParam->getIndex();
    if (modeChoice == 0)
        processFilter<chowdsp::ARPFilterType::Lowpass> (filter, buffer, notchParam);
    else if (modeChoice == 1)
        processFilter<chowdsp::ARPFilterType::Bandpass> (filter, buffer, notchParam);
    else if (modeChoice == 2)
        processFilter<chowdsp::ARPFilterType::Highpass> (filter, buffer, notchParam);
    else if (modeChoice == 3)
        processFilter<chowdsp::ARPFilterType::Notch> (filter, buffer, notchParam);
}
