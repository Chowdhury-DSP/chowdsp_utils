#include "BandSplitPlugin.h"

namespace
{
const juce::ParameterID freqTag = { "freq", 100 };
const juce::ParameterID orderTag = { "order", 100 };
const juce::ParameterID modeTag = { "mode", 100 };
} // namespace

BandSplitPlugin::BandSplitPlugin() = default;

void BandSplitPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const auto numChannels = getMainBusNumInputChannels();
    const auto spec = juce::dsp::ProcessSpec { sampleRate, (uint32_t) samplesPerBlock, (uint32_t) numChannels };
    filter1.prepare (spec);
    filter2.prepare (spec);
    filter4.prepare (spec);
    filter8.prepare (spec);
    filter12.prepare (spec);

    lowBuffer.setMaxSize (numChannels, samplesPerBlock);
    midBuffer.setMaxSize (numChannels, samplesPerBlock);
    highBuffer.setMaxSize (numChannels, samplesPerBlock);
}

void BandSplitPlugin::processAudioBlock (juce::AudioBuffer<float>& buffer)
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    // input buffer is used for low frequency signal
    auto&& bufferView = chowdsp::BufferView<float> { buffer };
    lowBuffer.setCurrentSize (numChannels, numSamples);
    highBuffer.setCurrentSize (numChannels, numSamples);

    const auto processFilter = [this, &bufferView] (auto& filter)
    {
        filter.setLowCrossoverFrequency (*state.params.freqLowParam);
        filter.setHighCrossoverFrequency (*state.params.freqHighParam);
        filter.processBlock (bufferView, lowBuffer, midBuffer, highBuffer);
    };

    const auto orderIndex = state.params.orderParam->getIndex();
    if (orderIndex == 0)
        processFilter (filter1);
    else if (orderIndex == 1)
        processFilter (filter2);
    else if (orderIndex == 2)
        processFilter (filter4);
    else if (orderIndex == 3)
        processFilter (filter8);
    else if (orderIndex == 4)
        processFilter (filter12);

    // mute bands if needed
    const auto modeIndex = state.params.modeParam->getIndex();
    if (modeIndex == 1) // solo low
    {
        chowdsp::BufferMath::applyGain (midBuffer, 0.0f);
        chowdsp::BufferMath::applyGain (highBuffer, 0.0f);
    }
    else if (modeIndex == 2) // solo mid
    {
        chowdsp::BufferMath::applyGain (lowBuffer, 0.0f);
        chowdsp::BufferMath::applyGain (highBuffer, 0.0f);
    }
    else if (modeIndex == 3) // solo high
    {
        chowdsp::BufferMath::applyGain (lowBuffer, 0.0f);
        chowdsp::BufferMath::applyGain (midBuffer, 0.0f);
    }

    // sum bands back together
    chowdsp::BufferMath::copyBufferData (lowBuffer, bufferView);
    chowdsp::BufferMath::addBufferData (midBuffer, bufferView);
    chowdsp::BufferMath::addBufferData (highBuffer, bufferView);
}
