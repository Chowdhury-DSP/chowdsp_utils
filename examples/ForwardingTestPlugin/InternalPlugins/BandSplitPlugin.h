#pragma once

#include <chowdsp_filters/chowdsp_filters.h>
#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

class BandSplitPlugin : public chowdsp::PluginBase<BandSplitPlugin>
{
public:
    BandSplitPlugin();

    static void addParameters (Parameters& params);

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override;

    bool hasEditor() const override { return false; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }

private:
    chowdsp::FloatParameter* freqParam = nullptr;
    chowdsp::ChoiceParameter* orderParam = nullptr;
    chowdsp::ChoiceParameter* modeParam = nullptr;

    chowdsp::LinkwitzRileyFilter<float, 1> filter1;
    chowdsp::LinkwitzRileyFilter<float, 2> filter2;
    chowdsp::LinkwitzRileyFilter<float, 4> filter4;
    chowdsp::LinkwitzRileyFilter<float, 8> filter8;
    chowdsp::LinkwitzRileyFilter<float, 12> filter12;

    chowdsp::Buffer<float> lowBuffer;
    chowdsp::Buffer<float> highBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BandSplitPlugin)
};
