#include "ModalReverbPlugin.h"

namespace
{
const juce::String pitchTag = "pitch";
const juce::String decayTag = "decay";
const juce::String mixTag = "mix";
const juce::String modModesTag = "mod_modes";
const juce::String modFreqTag = "mod_freq";
const juce::String modDepthTag = "mod_depth";
} // namespace

ModalReverbPlugin::ModalReverbPlugin()
{
    modalFilterBank.setModeAmplitudes (ModeParams::amps_real, ModeParams::amps_imag);

    pitchParam = vts.getRawParameterValue (pitchTag);
    decayParam = vts.getRawParameterValue (decayTag);
    mixParam = vts.getRawParameterValue (mixTag);
    modModesParam = vts.getRawParameterValue (modModesTag);
    modFreqParam = vts.getRawParameterValue (modFreqTag);
    modDepthParam = vts.getRawParameterValue (modDepthTag);
}

void ModalReverbPlugin::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;
    createPercentParameter (params, pitchTag, "Pitch", 0.5f);
    createPercentParameter (params, decayTag, "Decay", 0.5f);
    createPercentParameter (params, mixTag, "Mix", 0.5f);
    emplace_param<chowdsp::FloatParameter> (
        params,
        modModesTag,
        "Mod. Modes",
        createNormalisableRange (0.0f, 200.0f, 20.0f),
        0.0f,
        [] (float x) { return juce::String ((int) x); },
        &stringToFloatVal);
    createFreqParameter (params, modFreqTag, "Mod. Freq", 0.5f, 10.0f, 2.0f, 1.0f);
    createPercentParameter (params, modDepthTag, "Mod. Depth", 0.5f);
}

void ModalReverbPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    modalFilterBank.prepare (sampleRate, samplesPerBlock);

    modSine.prepare ({ sampleRate, (juce::uint32) samplesPerBlock, 1 });
    modBuffer.setSize (1, samplesPerBlock);

    mixer.prepare ({ sampleRate, (juce::uint32) samplesPerBlock, (juce::uint32) getTotalNumInputChannels() });
    mixer.setMixingRule (juce::dsp::DryWetMixingRule::linear);
}

void ModalReverbPlugin::processAudioBlock (juce::AudioBuffer<float>& buffer)
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    // make buffer mono
    for (int ch = 1; ch < numChannels; ++ch)
        buffer.addFrom (0, 0, buffer, ch, 0, numSamples);
    buffer.applyGain (1.0f / (float) numChannels);

    // push dry data int mixer
    mixer.setWetMixProportion (*mixParam);
    mixer.pushDrySamples (juce::dsp::AudioBlock<float> { buffer });

    // update mode frequencies
    const auto freqMult = std::pow (2.0f, 2.0f * *pitchParam - 1.0f);
    modalFilterBank.setModeFrequencies (ModeParams::freqs, freqMult);

    // update mode decay rates
    const auto decayFactor = std::pow (4.0f, 2.0f * *decayParam - 1.0f);
    modalFilterBank.setModeDecays (ModeParams::taus, ModeParams::analysisFs, decayFactor);

    // set up modulation data
    modBuffer.setSize (1, numSamples, false, false, true);
    modBuffer.clear();
    modSine.setFrequency (*modFreqParam);
    auto&& modBlock = juce::dsp::AudioBlock<float> { modBuffer };
    modSine.process (juce::dsp::ProcessContextReplacing<float> { modBlock });

    const auto numModesToMod = (int) modModesParam->load();
    if (numModesToMod == 0)
    {
        // process modal filter without modulation
        modalFilterBank.process (buffer);
    }
    else
    {
        // process modal filter with modulation
        const auto modDepth = modDepthParam->load();
        auto&& block = juce::dsp::AudioBlock<float> { buffer };
        const auto* modData = modBuffer.getReadPointer (0);
        modalFilterBank.processWithModulation (
            block,
            [modData, numModesToMod, freqMult, modDepth] (auto& mode, size_t vecModeIndex, size_t sampleIndex) {
                using Vec = decltype (modalFilterBank)::Vec;
                const auto modeScalarIndex = vecModeIndex * Vec::size;
                if (modeScalarIndex >= (size_t) numModesToMod)
                    return;

                auto modOffset = Vec (modData[sampleIndex]);
                if (modeScalarIndex + Vec::size > (size_t) numModesToMod)
                {
                    const auto numLeftoverModes = (size_t) numModesToMod - modeScalarIndex;
                    alignas (xsimd::default_arch::alignment()) float modVec[Vec::size] {};
                    std::fill (modVec, modVec + numLeftoverModes, modData[sampleIndex]);
                    modOffset = xsimd::load_aligned (modVec);
                }

                const auto freqOffset = xsimd::pow (Vec (2.0f), modOffset) * modDepth;
                mode.setFreq (xsimd::load_aligned (ModeParams::freqs + vecModeIndex * Vec::size) * freqMult * freqOffset);
            });
    }

    // split out of mono
    const auto& renderBuffer = modalFilterBank.getRenderBuffer();
    for (int ch = 0; ch < numChannels; ++ch)
        buffer.copyFrom (ch, 0, renderBuffer, 0, 0, numSamples);
    buffer.applyGain (juce::Decibels::decibelsToGain (-120.0f, -1000.0f));

    // mix dry/wet buffer
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
