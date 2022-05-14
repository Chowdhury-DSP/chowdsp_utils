#pragma once

#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

class AccessiblePlugin : public chowdsp::PluginBase<AccessiblePlugin>
{
public:
    AccessiblePlugin();

    static void addParameters (Parameters& params);

    void prepareToPlay (double, int) override {}
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>&) override {}

    juce::AudioProcessorEditor* createEditor() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AccessiblePlugin)
};
