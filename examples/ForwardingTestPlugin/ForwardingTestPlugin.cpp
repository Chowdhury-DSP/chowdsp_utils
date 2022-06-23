#include "ForwardingTestPlugin.h"
#include "PluginEditor.h"

namespace
{
constexpr int maxNumForwardingParameters = 20;

juce::String getForwardParamID (int paramNum)
{
    return "forward_param_" + juce::String (paramNum);
}
} // namespace

ForwardingTestPlugin::ForwardingTestPlugin()
{
    chowdsp::ParamUtils::loadParameterPointer (processorChoiceParameter, vts, processorChoiceParamID);

    for (int i = 0; i < maxNumForwardingParameters; ++i)
    {
        auto id = getForwardParamID (i);
        auto forwardedParam = std::make_unique<chowdsp::ForwardingParameter> (id, nullptr, "Blank");

        forwardedParam->setProcessor (&vts.processor);
        forwardedParams.add (forwardedParam.get());
        vts.processor.addParameter (forwardedParam.release());
    }

    vts.addParameterListener (processorChoiceParamID, this);
}

ForwardingTestPlugin::~ForwardingTestPlugin()
{
    vts.removeParameterListener (processorChoiceParamID, this);
}

void ForwardingTestPlugin::addParameters (Parameters& params)
{
    chowdsp::ParamUtils::emplace_param<chowdsp::ChoiceParameter> (params, processorChoiceParamID, "Processor Choice", juce::StringArray { "None", "Tone Generator", "Reverb" }, 0);
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

    for (auto* forwardingParam : forwardedParams)
        forwardingParam->setParam (nullptr);

    if (auto* newProcessor = getProcessorForIndex ((int) newValue))
    {
        int forwardParamIndex = 0;
        auto& processorParameters = newProcessor->getParameters();
        for (auto* param : processorParameters)
        {
            if (forwardParamIndex >= maxNumForwardingParameters)
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
