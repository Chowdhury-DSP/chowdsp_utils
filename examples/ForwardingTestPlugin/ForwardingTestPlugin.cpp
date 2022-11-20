#include "ForwardingTestPlugin.h"
#include "PluginEditor.h"

ForwardingTestPlugin::ForwardingTestPlugin() : forwardingParameters (vts)
{
    using namespace chowdsp::ParamUtils;
    chowdsp::ParamUtils::loadParameterPointer (processorChoiceParameter, vts, processorChoiceParamID);

    vts.addParameterListener (processorChoiceParamID, this);
}

ForwardingTestPlugin::~ForwardingTestPlugin()
{
    vts.removeParameterListener (processorChoiceParamID, this);
}

void ForwardingTestPlugin::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;
    emplace_param<chowdsp::ChoiceParameter> (params,
                                             chowdsp::ParameterID { processorChoiceParamID, 100 },
                                             "Processor Choice",
                                             juce::StringArray {
                                                 "None",
                                                 "Tone Generator",
                                                 "Reverb",
                                                 "Werner Filter",
                                                 "ARP Filter",
                                                 "Polygonal Oscillator",
                                                 "Band Split",
                                                 "Plate Reverb",
                                             },
                                             0);
}

void ForwardingTestPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    toneGenerator.prepareToPlay (sampleRate, samplesPerBlock);
    reverb.prepareToPlay (sampleRate, samplesPerBlock);
    wernerFilter.prepareToPlay (sampleRate, samplesPerBlock);
    arpFilter.prepareToPlay (sampleRate, samplesPerBlock);
    polygonalOsc.prepareToPlay (sampleRate, samplesPerBlock);
    bandSplit.prepareToPlay (sampleRate, samplesPerBlock);
    plateReverb.prepareToPlay (sampleRate, samplesPerBlock);
}

void ForwardingTestPlugin::processAudioBlock (juce::AudioBuffer<float>& buffer)
{
    juce::MidiBuffer midi;
    if (auto* processor = getProcessorForIndex (processorChoiceParameter->getIndex()))
    {
        processor->processBlock (buffer, midi);
    }
}

juce::AudioProcessorEditor* ForwardingTestPlugin::createEditor()
{
    return new PluginEditor (*this);
}

void ForwardingTestPlugin::parameterChanged (const juce::String& parameterID, float newValue)
{
    if (parameterID != ForwardingTestPlugin::processorChoiceParamID)
    {
        jassertfalse;
        return;
    }

    if (auto* newProcessor = getProcessorForIndex ((int) newValue))
    {
        auto& processorParameters = newProcessor->getParameters();
        forwardingParameters.setParameterRange (0,
                                                numForwardParameters,
                                                [&processorParameters] (int i) -> chowdsp::ParameterForwardingInfo
                                                {
                                                    if (auto* paramCast = dynamic_cast<juce::RangedAudioParameter*> (processorParameters[i]))
                                                        return { paramCast, paramCast->name };

                                                    return {};
                                                });
    }
    else
    {
        forwardingParameters.clearParameterRange (0, numForwardParameters);
    }
}

juce::AudioProcessor* ForwardingTestPlugin::getProcessorForIndex (int index)
{
    if (index == 1)
        return &toneGenerator;

    if (index == 2)
        return &reverb;

    if (index == 3)
        return &wernerFilter;

    if (index == 4)
        return &arpFilter;

    if (index == 5)
        return &polygonalOsc;

    if (index == 6)
        return &bandSplit;

    if (index == 7)
        return &plateReverb;

    return nullptr;
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ForwardingTestPlugin();
}
