#include "ModalReverbPlugin.h"

namespace
{
const juce::String pitchTag = "pitch";
const juce::String decayTag = "decay";
const juce::String mixTag = "mix";
} // namespace

ModalReverbPlugin::ModalReverbPlugin()
{
    modalFilterBank.setModeAmplitudes (ModeParams::amps_real, ModeParams::amps_imag);

    pitchParam = vts.getRawParameterValue (pitchTag);
    decayParam = vts.getRawParameterValue (decayTag);
    mixParam = vts.getRawParameterValue (mixTag);
}

void ModalReverbPlugin::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;
    createPercentParameter (params, pitchTag, "Pitch", 0.5f);
    createPercentParameter (params, decayTag, "Decay", 0.5f);
    createPercentParameter (params, mixTag, "Mix", 0.5f);
}

void ModalReverbPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    modalFilterBank.prepare (sampleRate, samplesPerBlock);

    mixer.prepare ({ sampleRate, (juce::uint32) samplesPerBlock, (juce::uint32) getTotalNumInputChannels() });
    mixer.setMixingRule (juce::dsp::DryWetMixingRule::linear);
}

void ModalReverbPlugin::processAudioBlock (juce::AudioBuffer<float>& buffer)
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    mixer.setWetMixProportion (*mixParam);
    mixer.pushDrySamples (juce::dsp::AudioBlock<float> { buffer });

    const auto freqMult = std::pow (2.0f, 2.0f * *pitchParam - 1.0f);
    modalFilterBank.setModeFrequencies (ModeParams::freqs, freqMult);

    const auto decayFactor = std::pow (4.0f, 2.0f * *decayParam - 1.0f);
    modalFilterBank.setModeDecays (ModeParams::taus, ModeParams::analysisFs, decayFactor);

    modalFilterBank.process (buffer);
    const auto& renderBuffer = modalFilterBank.getRenderBuffer();

    for (int ch = 0; ch < numChannels; ++ch)
        buffer.copyFrom (ch, 0, renderBuffer, 0, 0, numSamples);
    buffer.applyGain (juce::Decibels::decibelsToGain (-94.0f, -1000.0f));

    mixer.mixWetSamples (juce::dsp::AudioBlock<float> { buffer });
}

juce::AudioProcessorEditor* ModalReverbPlugin::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ModalReverbPlugin();
}
