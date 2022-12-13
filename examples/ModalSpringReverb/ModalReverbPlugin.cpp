#include "ModalReverbPlugin.h"

ModalReverbPlugin::ModalReverbPlugin()
{
    modalFilterBank.setModeAmplitudes (ModeParams::amps_real, ModeParams::amps_imag);
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
    const auto& params = state.params;

    // push dry data int mixer
    mixer.setWetMixProportion (params.mixParam->getCurrentValue());
    mixer.pushDrySamples (juce::dsp::AudioBlock<float> { buffer });

    // make buffer mono
    for (int ch = 1; ch < numChannels; ++ch)
        buffer.addFrom (0, 0, buffer, ch, 0, numSamples);
    buffer.applyGain (1.0f / (float) numChannels);

    // update mode frequencies
    const auto freqMult = std::pow (2.0f, params.pitchParam->getCurrentValue());
    modalFilterBank.setModeFrequencies (ModeParams::freqs, freqMult);

    // update mode decay rates
    const auto decayFactor = std::pow (4.0f, 2.0f * params.decayParam->getCurrentValue() - 1.0f);
    modalFilterBank.setModeDecays (ModeParams::taus, ModeParams::analysisFs, decayFactor);

    // set up modulation data
    modBuffer.setSize (1, numSamples, false, false, true);
    modBuffer.clear();
    modSine.setFrequency (params.modFreqParam->getCurrentValue());
    auto&& modBlock = juce::dsp::AudioBlock<float> { modBuffer };
    modSine.process (juce::dsp::ProcessContextReplacing<float> { modBlock });

    const auto numModesToMod = (int) params.modModesParam->getCurrentValue();
    if (numModesToMod == 0)
    {
        // process modal filter without modulation
        modalFilterBank.process (buffer);
    }
    else
    {
        // process modal filter with modulation
        const auto modDepth = params.modDepthParam->getCurrentValue();
        auto&& block = juce::dsp::AudioBlock<float> { buffer };
        const auto* modData = modBuffer.getReadPointer (0);
        modalFilterBank.processWithModulation (
            block,
            [modData, numModesToMod, freqMult, modDepth] (auto& mode, size_t vecModeIndex, int sampleIndex)
            {
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
        chowdsp::BufferMath::copyBufferChannels (renderBuffer, buffer, 0, ch);
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
