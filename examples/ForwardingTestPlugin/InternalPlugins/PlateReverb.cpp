#include "PlateReverb.h"

PlateReverb::PlateReverb() = default;

void PlateReverb::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (sampleRate, samplesPerBlock);
    for (auto& diff : diffusion)
        diff.prepare ((float) sampleRate);
    tank.prepare ((float) sampleRate);

    mixer.prepare ({ sampleRate, (uint32_t) samplesPerBlock, 2 });
    mixer.setMixingRule (juce::dsp::DryWetMixingRule::sin3dB);
}

void PlateReverb::processAudioBlock (juce::AudioBuffer<float>& buffer)
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    mixer.setWetMixProportion (*state.params.mix);
    mixer.pushDrySamples (buffer);

    const auto inputDiffusionPercent = state.params.inputDiffusion->getCurrentValue();
    const auto diffMult1 = 0.25f + inputDiffusionPercent * 0.5f;
    const auto diffMult2 = 0.05f + inputDiffusionPercent * 0.5f;

    for (int ch = 0; ch < numChannels; ++ch)
    {
        using InputNetworkConfig = chowdsp::Reverb::Dattorro::DefaultInputNetworkConfig<>;
        InputNetworkConfig::setInputDiffusionParameters (diffusion[ch], diffMult1, diffMult2);

        auto* data = buffer.getWritePointer (ch);
        for (int n = 0; n < numSamples; ++n)
            data[n] = diffusion[ch].processSample (data[n]);
    }

    using TankConfig = chowdsp::Reverb::Dattorro::DefaultTankNetworkConfig<>;
    tank.setDecayAmount (0.5f * state.params.decay->getCurrentValue());
    tank.setDampingFrequency (state.params.damping->getCurrentValue());
    TankConfig::setDecayDiffusion1Parameters (tank, state.params.decayDiffusion->getCurrentValue());

    const auto leftCh = 0;
    const auto rightCh = 1 % numChannels;

    auto x = buffer.getArrayOfWritePointers();
    for (int n = 0; n < numSamples; ++n)
    {
        std::tie (x[leftCh][n], x[rightCh][n]) = tank.processSample (x[leftCh][n], x[rightCh][n]);
    }

    mixer.mixWetSamples (buffer);
}
