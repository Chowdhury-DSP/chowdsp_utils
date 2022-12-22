#include "PolygonalOscPlugin.h"

PolygonalOscPlugin::PolygonalOscPlugin() = default;

void PolygonalOscPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const auto spec = juce::dsp::ProcessSpec { sampleRate, (uint32_t) samplesPerBlock, (uint32_t) getMainBusNumInputChannels() };
    oscillator.prepare (spec);
    gain.prepare (spec);
    gain.setRampDurationSeconds (0.05);
}

void PolygonalOscPlugin::processAudioBlock (juce::AudioBuffer<float>& buffer)
{
    buffer.clear();

    oscillator.setFrequency (*state.params.freqParam);
    oscillator.setOrder (*state.params.orderParam);
    oscillator.setTeeth (*state.params.teethParam);
    oscillator.processBlock (buffer);

    gain.setGainDecibels (*state.params.gainParam);
    gain.process (buffer);
}
