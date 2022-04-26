#include "SimpleEQPlugin.h"

namespace
{
juce::String getTagForBand (int bandIndex, const juce::String& tag)
{
    return "band" + juce::String (bandIndex) + "_" + tag;
}

juce::String getNameForBand (int bandIndex, const juce::String& name)
{
    return "Band " + juce::String (bandIndex) + " " + name;
}

const juce::String freqTag = "filter_freq";
const juce::String qTag = "filter_q";
const juce::String gainTag = "filter_gain";
const juce::String typeTag = "filter_type";
const juce::String onOffTag = "filter_on_off";
const juce::String linPhaseModeTag = "linear_phase_mode";

const auto eqTypeChoices = juce::StringArray {
    "1-Pole HPF",
    "2-Pole HPF",
    "4-Pole HPF",
    "8-Pole HPF",
    "8-Pole Cheby. HPF",
    "Low-Shelf",
    "Bell",
    "Notch",
    "Deep Notch",
    "High-Shelf",
    "1-Pole LPF",
    "2-Pole LPF",
    "4-Pole LPF",
    "8-Pole LPF",
    "8-Pole Cheby. LPF",
};
} // namespace

SimpleEQPlugin::SimpleEQPlugin()
{
    for (size_t i = 0; i < EQParams::numBands; ++i)
    {
        bandFreqHz[i] = vts.getRawParameterValue (getTagForBand ((int) i, freqTag));
        bandQ[i] = vts.getRawParameterValue (getTagForBand ((int) i, qTag));
        bandGainDB[i] = vts.getRawParameterValue (getTagForBand ((int) i, gainTag));
        bandType[i] = vts.getRawParameterValue (getTagForBand ((int) i, typeTag));
        bandOnOff[i] = vts.getRawParameterValue (getTagForBand ((int) i, onOffTag));
    }

    linPhaseModeOn = vts.getRawParameterValue (linPhaseModeTag);

    linPhaseEQ.updatePrototypeEQParameters = [] (auto& eq, auto& eqParams) { eq.setParameters (eqParams); };
}

void SimpleEQPlugin::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;
    auto addQParam = [&params] (const juce::String& tag, const juce::String& name) {
        emplace_param<VTSParam> (params, tag, name, juce::String(), createNormalisableRange (0.1f, 10.0f, 0.7071f), 0.7071f, &floatValToString, &stringToFloatVal);
    };

    for (int i = 0; i < (int) EQParams::numBands; ++i)
    {
        createFreqParameter (params, getTagForBand (i, freqTag), getNameForBand (i, "Freq."), 20.0f, 20000.0f, 2000.0f, 1000.0f);
        addQParam (getTagForBand (i, qTag), getNameForBand (i, "Q"));
        createGainDBParameter (params, getTagForBand (i, gainTag), getNameForBand (i, "Gain"), -18.0f, 18.0f, 0.0f);
        emplace_param<juce::AudioParameterChoice> (params, getTagForBand (i, typeTag), getNameForBand (i, "Type"), eqTypeChoices, 3);
        emplace_param<juce::AudioParameterBool> (params, getTagForBand (i, onOffTag), getNameForBand (i, "On/Off"), false);
    }

    emplace_param<juce::AudioParameterBool> (params, linPhaseModeTag, "Linear Phase", false);
}

void SimpleEQPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const auto&& eqParams = makeEQParams();
    const auto spec = juce::dsp::ProcessSpec { sampleRate, (juce::uint32) samplesPerBlock, (juce::uint32) getTotalNumInputChannels() };

    protoEQ.setParameters (eqParams);
    protoEQ.prepare (spec);

    linPhaseEQ.prepare (spec, makeEQParams()); // prepare the linear phase EQ with some initial parameters

    // If linear phase mode will always be on, this is the place to report latency to the host!
    // setLatencySamples (linPhaseEQ.getLatencySamples());
}

PrototypeEQ::Params SimpleEQPlugin::makeEQParams() const
{
    EQParams params {};
    for (size_t i = 0; i < EQParams::numBands; ++i)
    {
        params.bands[i] = EQParams::BandParams {
            *bandFreqHz[i],
            *bandQ[i],
            *bandGainDB[i],
            (int) *bandType[i],
            *bandOnOff[i] == 1.0f,
        };
    }

    return params;
}

void SimpleEQPlugin::setEQParams()
{
    const auto&& eqParams = makeEQParams();
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

juce::AudioProcessorEditor* SimpleEQPlugin::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleEQPlugin();
}
