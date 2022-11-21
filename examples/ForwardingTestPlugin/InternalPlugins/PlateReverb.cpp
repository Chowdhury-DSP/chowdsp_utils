#include "PlateReverb.h"

namespace
{
const juce::ParameterID inputDiffusionTag { "input_diffusion", 100 };
const juce::ParameterID decayTag { "decay", 100 };
const juce::ParameterID decayDiffusionTag { "decay_diffusion", 100 };
const juce::ParameterID dampingTag { "damping", 100 };
const juce::ParameterID mixTag { "mix", 100 };
} // namespace

PlateReverb::PlateReverb()
{
    using namespace chowdsp::ParamUtils;
    loadParameterPointer (inputDiffusionParam, vts, inputDiffusionTag);
    loadParameterPointer (decayParam, vts, decayTag);
    loadParameterPointer (decayDiffusionParam, vts, decayDiffusionTag);
    loadParameterPointer (dampingParam, vts, dampingTag);
    loadParameterPointer (mixParam, vts, mixTag);
}

void PlateReverb::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;
    createPercentParameter (params, inputDiffusionTag, "Input Diffusion", 0.5f);
    createPercentParameter (params, decayTag, "Decay", 0.5f);
    createPercentParameter (params, decayDiffusionTag, "Decay Diffusion", 0.5f);
    createFreqParameter (params, dampingTag, "Damping", 2000.0f, 20000.0f, 10000.0f, 10000.0f);
    createPercentParameter (params, mixTag, "Mix", 0.5f);
}

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

    mixer.setWetMixProportion (mixParam->getCurrentValue());
    mixer.pushDrySamples (buffer);

    const auto inputDiffusionPercent = inputDiffusionParam->getCurrentValue();
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
    tank.setDecayAmount (0.5f * decayParam->getCurrentValue());
    tank.setDampingFrequency (dampingParam->getCurrentValue());
    TankConfig::setDecayDiffusion1Parameters (tank, decayDiffusionParam->getCurrentValue());

    const auto leftCh = 0;
    const auto rightCh = 1 % numChannels;

    auto x = buffer.getArrayOfWritePointers();
    for (int n = 0; n < numSamples; ++n)
    {
        std::tie (x[leftCh][n], x[rightCh][n]) = tank.processSample (x[leftCh][n], x[rightCh][n]);
    }

    mixer.mixWetSamples (buffer);
}
