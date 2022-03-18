#include "LinearPhaseEQPlugin.h"

namespace
{
const juce::String lowCutFreqTag = "low_cut_freq";
const juce::String lowCutQTag = "low_cut_q";
const juce::String peakingFilterFreqTag = "peak_filter_freq";
const juce::String peakingFilterQTag = "peak_filter_q";
const juce::String peakingFilterGainTag = "peak_filter_gain";
const juce::String highCutFreqTag = "high_cut_freq";
const juce::String highCutQTag = "high_cut_q";
const juce::String linPhaseModeTag = "linear_phase_mode";
} // namespace

LinearPhaseEQPlugin::LinearPhaseEQPlugin()
{
    lowCutFreqHz = vts.getRawParameterValue (lowCutFreqTag);
    lowCutQ = vts.getRawParameterValue (lowCutQTag);
    peakingFilterFreqHz = vts.getRawParameterValue (peakingFilterFreqTag);
    peakingFilterQ = vts.getRawParameterValue (peakingFilterQTag);
    peakingFilterGainDB = vts.getRawParameterValue (peakingFilterGainTag);
    highCutFreqHz = vts.getRawParameterValue (highCutFreqTag);
    highCutQ = vts.getRawParameterValue (highCutQTag);
    linPhaseModeOn = vts.getRawParameterValue (linPhaseModeTag);

    linPhaseEQ.updatePrototypeEQParameters = [] (auto& eq, auto& eqParams)
    { eq.setParameters (eqParams, true); };
}

void LinearPhaseEQPlugin::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;
    createFreqParameter (params, lowCutFreqTag, "Low Cut Freq.", 20.0f, 1000.0f, 400.0f, 20.0f);
    createFreqParameter (params, peakingFilterFreqTag, "Bell Freq.", 20.0f, 20000.0f, 2000.0f, 1000.0f);
    createFreqParameter (params, highCutFreqTag, "High Cut Freq.", 1000.0f, 20000.0f, 4000.0f, 20000.0f);

    auto addQParam = [&params] (const juce::String& tag, const juce::String& name)
    {
        emplace_param<VTSParam> (params, tag, name, juce::String(), createNormalisableRange (0.1f, 10.0f, 0.7071f), 0.7071f, &floatValToString, &stringToFloatVal);
    };

    addQParam (lowCutQTag, "Low Cut Q");
    addQParam (peakingFilterQTag, "Bell Q");
    addQParam (highCutQTag, "High Cut Q");

    createGainDBParameter (params, peakingFilterGainTag, "Bell Gain", -18.0f, 18.0f, 0.0f);

    emplace_param<juce::AudioParameterBool> (params, linPhaseModeTag, "Linear Phase", false);
}

void LinearPhaseEQPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const auto spec = juce::dsp::ProcessSpec { sampleRate, (juce::uint32) samplesPerBlock, 1 };
    protoEQ.prepare (spec);
    linPhaseEQ.prepare (spec, makeEQParams());

    // If linear phase mode will always be on, this is the place to report latency to the host!
    // setLatencySamples (linPhaseEQ.getLatencySamples());

    setEQParams (true);
}

PrototypeEQ::Params LinearPhaseEQPlugin::makeEQParams() const
{
    return {
        *lowCutFreqHz,
        *lowCutQ,
        *peakingFilterFreqHz,
        *peakingFilterQ,
        *peakingFilterGainDB,
        *highCutFreqHz,
        *highCutQ,
    };
}

void LinearPhaseEQPlugin::setEQParams (bool force)
{
    const auto&& eqParams = makeEQParams();
    protoEQ.setParameters (eqParams, force);
    linPhaseEQ.setParameters (eqParams);
}

void LinearPhaseEQPlugin::processAudioBlock (juce::AudioBuffer<float>& buffer)
{
    // Warning: only processing in mono for now!
    buffer.copyFrom (0, 0, buffer, 1, 0, buffer.getNumSamples());

    setEQParams();
    if (*linPhaseModeOn == 0)
    {
        protoEQ.processBlock (buffer);
    }
    else
    {
        auto&& block = juce::dsp::AudioBlock<float> { buffer }.getSingleChannelBlock (0);
        linPhaseEQ.process (juce::dsp::ProcessContextReplacing<float> { block });
    }

    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
        buffer.copyFrom (ch, 0, buffer, 0, 0, buffer.getNumSamples());
}

juce::AudioProcessorEditor* LinearPhaseEQPlugin::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LinearPhaseEQPlugin();
}
