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
    emplace_param<chowdsp::ChoiceParameter> (params, processorChoiceParamID, "Processor Choice", juce::StringArray { "None", "Tone Generator", "Reverb" }, 0);

    ForwardingParams::addParameters (params);
}

void ForwardingTestPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    toneGenerator.prepareToPlay (sampleRate, samplesPerBlock);
    reverb.prepareToPlay (sampleRate, samplesPerBlock);
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

    auto& forwardedParams = forwardingParameters.getForwardedParameters();
    for (auto* forwardingParam : forwardedParams)
        forwardingParam->setParam (nullptr);

    if (auto* newProcessor = getProcessorForIndex ((int) newValue))
    {
        size_t forwardParamIndex = 0;
        auto& processorParameters = newProcessor->getParameters();
        for (auto* param : processorParameters)
        {
            if (forwardParamIndex >= forwardedParams.size())
            {
                // this processor has too many parameters!
                jassertfalse;
                break;
            }

            if (auto* paramCast = dynamic_cast<juce::RangedAudioParameter*> (param))
                forwardedParams[forwardParamIndex++]->setParam (paramCast, paramCast->name);
        }
    }
}

juce::AudioProcessor* ForwardingTestPlugin::getProcessorForIndex (int index)
{
    if (index == 1)
        return &toneGenerator;

    if (index == 2)
        return &reverb;

    return nullptr;
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ForwardingTestPlugin();
}
