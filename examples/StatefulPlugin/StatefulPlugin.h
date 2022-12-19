#pragma once

#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

struct LevelParams : chowdsp::ParamHolder
{
    LevelParams() : chowdsp::ParamHolder ("Level")
    {
        add (percent, gain);
    }

    chowdsp::PercentParameter::Ptr percent { juce::ParameterID { "percent", 100 }, "Percent" };
    chowdsp::GainDBParameter::Ptr gain { juce::ParameterID { "gain", 100 }, "Gain", juce::NormalisableRange { -30.0f, 0.0f }, 0.0f };
};

struct PluginParameterState : chowdsp::ParamHolder
{
    PluginParameterState()
    {
        add (levelParams, mode, onOff);
    }

    LevelParams levelParams;
    chowdsp::ChoiceParameter::Ptr mode { juce::ParameterID { "mode", 100 }, "Mode", juce::StringArray { "Percent", "Gain", "Percent / Gain", "Gain / Percent" }, 2 };
    chowdsp::BoolParameter::Ptr onOff { juce::ParameterID { "on_off", 100 }, "On/Off", true };
};

struct PluginNonParameterState
{
    chowdsp::StateValue<juce::Point<int>> editorBounds { "editor_bounds", { 300, 500 } };
};

using State = chowdsp::PluginStateImpl<PluginParameterState, PluginNonParameterState>;

class StatefulPlugin : public chowdsp::PluginBase<State>
{
public:
    StatefulPlugin();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override;

    juce::AudioProcessorEditor* createEditor() override;

    juce::UndoManager undoManager { 3000 };

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StatefulPlugin)
};
