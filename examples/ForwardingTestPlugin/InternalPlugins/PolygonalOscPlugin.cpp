#include "PolygonalOscPlugin.h"

namespace
{
const juce::String gainTag = "gain";
const juce::String freqTag = "freq";
const juce::String orderTag = "order";
const juce::String teethTag = "teeth";
} // namespace

PolygonalOscPlugin::PolygonalOscPlugin()
{
    using namespace chowdsp::ParamUtils;
    loadParameterPointer (gainDBParam, vts, gainTag);
    loadParameterPointer (freqHzParam, vts, freqTag);
    loadParameterPointer (orderParam, vts, orderTag);
    loadParameterPointer (teethParam, vts, teethTag);
}

void PolygonalOscPlugin::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;
    createGainDBParameter (params, { gainTag, 100 }, "Gain", -30.0f, 0.0f, -24.0f);
    createFreqParameter (params, { freqTag, 100 }, "Frequency", 20.0f, 2000.0f, 200.0f, 440.0f);
    emplace_param<chowdsp::FloatParameter> (params, juce::ParameterID { orderTag, 100 }, "Order", createNormalisableRange (2.01f, 10.0f, 4.0f), 3.5f, &floatValToString, &stringToFloatVal);
    createPercentParameter (params, { teethTag, 100 }, "Teeth", 0.0f);
}

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

    oscillator.setFrequency (freqHzParam->getCurrentValue());
    oscillator.setOrder (orderParam->getCurrentValue());
    oscillator.setTeeth (teethParam->getCurrentValue());
    oscillator.processBlock (buffer);

    gain.setGainDecibels (gainDBParam->getCurrentValue());
    gain.process (buffer);
}
