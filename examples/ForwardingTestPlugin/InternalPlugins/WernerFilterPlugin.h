#pragma once

#include <chowdsp_filters/chowdsp_filters.h>
#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

class WernerFilterPlugin : public chowdsp::PluginBase<WernerFilterPlugin>
{
public:
    WernerFilterPlugin();

    static void addParameters (Parameters& params);

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override;

    bool hasEditor() const override { return false; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }

private:
    chowdsp::FloatParameter* freqParam = nullptr;
    chowdsp::FloatParameter* resonanceParam = nullptr;
    chowdsp::FloatParameter* dampingParam = nullptr;
    chowdsp::SmoothedBufferValue<float> morphParam;
    chowdsp::ChoiceParameter* modeParam = nullptr;

    chowdsp::WernerFilter filter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WernerFilterPlugin)
};
