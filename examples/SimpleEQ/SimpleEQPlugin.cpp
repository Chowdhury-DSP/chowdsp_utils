#include "SimpleEQPlugin.h"

namespace
{
juce::String getTagForBand (int bandIndex, const juce::String& tag)
{
    return "band" + juce::String (bandIndex) + "_" + tag;
}

const juce::String freqTag = "filter_freq";
const juce::String qTag = "filter_q";
const juce::String gainTag = "filter_gain";
const juce::String typeTag = "filter_type";
const juce::String linPhaseModeTag = "linear_phase_mode";

const auto eqTypeChoices = juce::StringArray {
    "1-Pole HPF",
    "2-Pole HPF",
    "Bell",
    "Low-Shelf",
    "High-Shelf",
    "1-Pole LPF",
    "2-Pole LPF",
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
    }

    linPhaseModeOn = vts.getRawParameterValue (linPhaseModeTag);

    linPhaseEQ.updatePrototypeEQParameters = [] (auto& eq, auto& eqParams)
    { eq.setParameters (eqParams); };
}

void SimpleEQPlugin::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;
    auto addQParam = [&params] (const juce::String& tag, const juce::String& name)
    {
        emplace_param<VTSParam> (params, tag, name, juce::String(), createNormalisableRange (0.1f, 10.0f, 0.7071f), 0.7071f, &floatValToString, &stringToFloatVal);
    };

    for (int i = 0; i < (int) EQParams::numBands; ++i)
    {
        createFreqParameter (params, getTagForBand (i, freqTag), "Band " + juce::String (i) + " Freq.", 20.0f, 20000.0f, 2000.0f, 1000.0f);
        addQParam (getTagForBand (i, qTag), "Band " + juce::String (i) + " Q");
        createGainDBParameter (params, getTagForBand (i, gainTag), "Band " + juce::String (i) + " Gain", -18.0f, 18.0f, 0.0f);
        emplace_param<juce::AudioParameterChoice> (params, getTagForBand (i, typeTag), "Band " + juce::String (i) + " Type", eqTypeChoices, 2);
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
    return {
        EQParams::BandParams {
            *bandFreqHz[0],
            *bandQ[0],
            *bandGainDB[0],
            (int) *bandType[0],
        },
        EQParams::BandParams {
            *bandFreqHz[1],
            *bandQ[1],
            *bandGainDB[1],
            (int) *bandType[1],
        },
        EQParams::BandParams {
            *bandFreqHz[2],
            *bandQ[2],
            *bandGainDB[2],
            (int) *bandType[2],
        }
    };
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
        // For A/B testing purposes: process the prototype EQ
        protoEQ.processBlock (buffer);
    }
    else
    {
        // Here we are: processing the linear phase EQ!
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
