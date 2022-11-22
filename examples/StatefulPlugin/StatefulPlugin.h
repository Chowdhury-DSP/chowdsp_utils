#pragma once

#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

struct PluginParameterState
{
    chowdsp::SmartPointer<chowdsp::PercentParameter> percent { "percent", "Percent" };
    chowdsp::SmartPointer<chowdsp::GainDBParameter> gain { "gain", "Gain", juce::NormalisableRange { -30.0f, 0.0f }, 0.0f };
};

using State = chowdsp::PluginState<PluginParameterState>;

class StatefulPlugin : public chowdsp::PluginBaseWithState<State>
{
public:
    StatefulPlugin();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override;

    juce::AudioProcessorEditor* createEditor() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StatefulPlugin)
};
