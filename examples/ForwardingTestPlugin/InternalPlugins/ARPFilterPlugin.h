#pragma once

#include <chowdsp_filters/chowdsp_filters.h>
#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

class ARPFilterPlugin : public chowdsp::PluginBase<ARPFilterPlugin>
{
public:
    ARPFilterPlugin();

    static void addParameters (Parameters& params);

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override;

    bool hasEditor() const override { return false; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }

private:
    chowdsp::FloatParameter* freqParam = nullptr;
    chowdsp::FloatParameter* qParam = nullptr;
    chowdsp::BoolParameter* limitModeParam = nullptr;
    chowdsp::SmoothedBufferValue<float> notchParam;
    chowdsp::ChoiceParameter* modeParam = nullptr;

    chowdsp::ARPFilter<float> filter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARPFilterPlugin)
};
