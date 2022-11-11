#include "PlateReverb.h"

namespace
{
const juce::ParameterID inputDiffusionTag { "input_diffusion", 100 };
const juce::ParameterID decayTag { "decay", 100 };
const juce::ParameterID decayDiffusionTag { "decay_diffusion", 100 };
} // namespace

PlateReverb::PlateReverb()
{
    using namespace chowdsp::ParamUtils;
    loadParameterPointer (inputDiffusionParam, vts, inputDiffusionTag);
    loadParameterPointer (decayParam, vts, inputDiffusionTag);
    loadParameterPointer (decayDiffusionParam, vts, inputDiffusionTag);
}

void PlateReverb::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;
    createPercentParameter (params, inputDiffusionTag, "Input Diffusion", 0.5f);
    createPercentParameter (params, inputDiffusionTag, "Decay", 0.5f);
    createPercentParameter (params, inputDiffusionTag, "Decay Diffusion", 0.5f);
}

void PlateReverb::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);
    reverb.prepare ((float) sampleRate);
}

void PlateReverb::processAudioBlock (juce::AudioBuffer<float>& buffer)
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    const auto inputDiffusionPercent = inputDiffusionParam->getCurrentValue();
    ReverbConfig::setInputDiffusionParameters (reverb, 0.5f + inputDiffusionPercent * 0.3f, 0.25f + inputDiffusionPercent * 0.4f);

    reverb.setDecayAmount (0.5f * decayParam->getCurrentValue());

    ReverbConfig::setDecayDiffusion1Parameters (reverb, decayDiffusionParam->getCurrentValue());

    const auto leftCh = 0;
    const auto rightCh = 1 % numChannels;

    auto x = buffer.getArrayOfWritePointers();
    for (int n = 0; n < numSamples; ++n)
    {
        const auto in = 0.5f * (x[leftCh][n] + x[rightCh][n]);
        const auto out = reverb.processSample (in);
        x[leftCh][n] = x[rightCh][n] = out;
    }
}
