#include "SimpleEQPlugin.h"
#include "PluginEditor.h"

namespace
{
const juce::String linPhaseModeTag = "linear_phase_mode";

const auto eqTypeChoices = juce::StringArray {
    "1-Pole HPF",
    "2-Pole HPF",
    "4-Pole HPF",
    "8-Pole HPF",
    "8-Pole Cheby. HPF",
    "12-Pole Elliptic HPF",
    "Low-Shelf",
    "Bell",
    "Notch",
    "High-Shelf",
    "1-Pole LPF",
    "2-Pole LPF",
    "4-Pole LPF",
    "8-Pole LPF",
    "8-Pole Cheby. LPF",
    "12-Pole Elliptic LPF",
};
} // namespace

SimpleEQPlugin::SimpleEQPlugin()
{
    PrototypeEQ::EQParams::initialiseEQParameters (vts, eqParamsHandles);
    linPhaseModeOn = vts.getRawParameterValue (linPhaseModeTag);

    linPhaseEQ.updatePrototypeEQParameters = [] (auto& eq, auto& eqParams) { eq.setParameters (eqParams); };

    vts.addParameterListener (linPhaseModeTag, this);
}

SimpleEQPlugin::~SimpleEQPlugin()
{
    vts.removeParameterListener (linPhaseModeTag, this);
}

void SimpleEQPlugin::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;
    PrototypeEQ::EQParams::addEQParameters (params, PrototypeEQ::EQParams::defaultEQParamPrefix, eqTypeChoices, 7);
    emplace_param<juce::AudioParameterBool> (params, linPhaseModeTag, "Linear Phase", false);
}

void SimpleEQPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const auto&& eqParams = getEQParams();
    const auto spec = juce::dsp::ProcessSpec { sampleRate, (juce::uint32) samplesPerBlock, (juce::uint32) getTotalNumInputChannels() };

    protoEQ.setParameters (eqParams);
    protoEQ.prepare (spec);

    linPhaseEQ.prepare (spec, getEQParams()); // prepare the linear phase EQ with some initial parameters

    // make sure the reported latency is up-to-date
    parameterChanged (linPhaseModeTag, *linPhaseModeOn);
}

PrototypeEQ::Params SimpleEQPlugin::getEQParams() const
{
    return PrototypeEQ::EQParams::getEQParameters (eqParamsHandles);
}

void SimpleEQPlugin::loadEQParams (const PrototypeEQ::Params& params)
{
    PrototypeEQ::EQParams::loadEQParameters (params, vts);
}

void SimpleEQPlugin::setEQParams()
{
    const auto&& eqParams = getEQParams();
    protoEQ.setParameters (eqParams);
    linPhaseEQ.setParameters (eqParams);
}

void SimpleEQPlugin::processAudioBlock (juce::AudioBuffer<float>& buffer)
{
    // update the linear phase EQ parameters. This operation is safe to do from
    // any thread, but we'll do it on the audio thread here.
    setEQParams();

    if (*linPhaseModeOn == 0)
    {
        // Not in linear phase mode, so just process the regular EQ
        protoEQ.processBlock (buffer);
    }
    else
    {
        // Linear phase mode is on: processing the linear phase EQ here!
        auto&& block = juce::dsp::AudioBlock<float> { buffer };
        linPhaseEQ.process (juce::dsp::ProcessContextReplacing<float> { block });
    }
}

void SimpleEQPlugin::parameterChanged (const juce::String& parameterID, float newValue)
{
    if (parameterID == linPhaseModeTag)
        setLatencySamples (newValue == 1.0f ? linPhaseEQ.getLatencySamples() : 0);
}

juce::AudioProcessorEditor* SimpleEQPlugin::createEditor()
{
    return new PluginEditor (*this); // juce::GenericAudioProcessorEditor (*this);
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleEQPlugin();
}
