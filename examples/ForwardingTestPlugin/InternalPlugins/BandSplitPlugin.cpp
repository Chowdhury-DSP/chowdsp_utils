#include "BandSplitPlugin.h"

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

    for (auto& buffer : outBuffers)
        buffer.setMaxSize (numChannels, samplesPerBlock);
}

void BandSplitPlugin::processAudioBlock (juce::AudioBuffer<float>& buffer)
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    // input buffer is used for low frequency signal
    auto&& bufferView = chowdsp::BufferView<float> { buffer };
    for (auto& outBuffer : outBuffers)
        outBuffer.setCurrentSize (numChannels, numSamples);

    const auto processFilter = [this, &bufferView] (auto& filter)
    {
        filter.setCrossoverFrequency (0, *state.params.freqLowParam);
        filter.setCrossoverFrequency (1, *state.params.freqMidParam);
        filter.setCrossoverFrequency (2, *state.params.freqHighParam);

        filter.processBlock (bufferView, { outBuffers[0], outBuffers[1], outBuffers[2], outBuffers[3] });
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
        for (auto bufferIndex : { 1, 2, 3 })
            outBuffers[(size_t) bufferIndex].clear();
    }
    else if (modeIndex == 2) // solo mid-low
    {
        for (auto bufferIndex : { 0, 2, 3 })
            outBuffers[(size_t) bufferIndex].clear();
    }
    else if (modeIndex == 3) // solo mid-high
    {
        for (auto bufferIndex : { 0, 1, 3 })
            outBuffers[(size_t) bufferIndex].clear();
    }
    else if (modeIndex == 4) // solo high
    {
        for (auto bufferIndex : { 0, 1, 2 })
            outBuffers[(size_t) bufferIndex].clear();
    }

    // sum bands back together
    chowdsp::BufferMath::copyBufferData (outBuffers[0], bufferView);
    for (size_t i = 1; i < outBuffers.size(); ++i)
        chowdsp::BufferMath::addBufferData (outBuffers[i], bufferView);
}
