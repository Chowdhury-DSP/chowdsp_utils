#pragma once

#include <chowdsp_sources/chowdsp_sources.h>
#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

class PolygonalOscPlugin : public chowdsp::PluginBase<PolygonalOscPlugin>
{
public:
    PolygonalOscPlugin();

    static void addParameters (Parameters& params);

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override;

    bool hasEditor() const override { return false; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }

private:
    chowdsp::FloatParameter* gainDBParam = nullptr;
    chowdsp::FloatParameter* freqHzParam = nullptr;
    chowdsp::FloatParameter* orderParam = nullptr;
    chowdsp::FloatParameter* teethParam = nullptr;

    chowdsp::experimental::PolygonalOscillator<float> oscillator;
    chowdsp::Gain<float> gain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolygonalOscPlugin)
};
