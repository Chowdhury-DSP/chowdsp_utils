#include "BandSplitPlugin.h"

namespace
{
const juce::ParameterID freqTag = { "freq", 100 };
const juce::ParameterID orderTag = { "order", 100 };
const juce::ParameterID modeTag = { "mode", 100 };
} // namespace

BandSplitPlugin::BandSplitPlugin()
{
    using namespace chowdsp::ParamUtils;
    loadParameterPointer (freqParam, vts, freqTag);
    loadParameterPointer (orderParam, vts, orderTag);
    loadParameterPointer (modeParam, vts, modeTag);
}

void BandSplitPlugin::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;
    createFreqParameter (params, freqTag, "Crossover Frequency", 20.0f, 20000.0f, 1000.0f, 1000.0f);
    emplace_param<chowdsp::ChoiceParameter> (params, orderTag, "Order", juce::StringArray { "1", "2", "4", "8", "12" }, 0);
    emplace_param<chowdsp::ChoiceParameter> (params, modeTag, "Mode", juce::StringArray { "Through", "Mute Low", "Mute High" }, 0);
}

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
    highBuffer.setMaxSize (numChannels, samplesPerBlock);
}

void BandSplitPlugin::processAudioBlock (juce::AudioBuffer<float>& buffer)
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    // input buffer is used for low frequency signal
    auto&& bufferView = chowdsp::BufferView { buffer };
    lowBuffer.setCurrentSize (numChannels, numSamples);
    highBuffer.setCurrentSize (numChannels, numSamples);

    const auto processFilter = [this, &bufferView] (auto& filter)
    {
        filter.setCrossoverFrequency (freqParam->getCurrentValue());
        filter.processBlock (bufferView, lowBuffer, highBuffer);
    };

    const auto orderIndex = orderParam->getIndex();
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
    const auto modeIndex = modeParam->getIndex();
    if (modeIndex == 1)
        chowdsp::BufferMath::applyGain (lowBuffer, 0.0f);
    else if (modeIndex == 2)
        chowdsp::BufferMath::applyGain (highBuffer, 0.0f);

    // sum bands back together
    chowdsp::BufferMath::copyBufferData (lowBuffer, bufferView);
    chowdsp::BufferMath::addBufferData (highBuffer, bufferView);
}
