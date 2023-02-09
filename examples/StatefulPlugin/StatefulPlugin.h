#pragma once

#include <chowdsp_plugin_base/chowdsp_plugin_base.h>
#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>

struct LevelParams : chowdsp::ParamHolder
{
    LevelParams() : chowdsp::ParamHolder ("Level")
    {
        add (percent, gain);
    }

    static constexpr auto percentID = chowdsp::StringLiteral { "percent" };
    chowdsp::PercentParameter::Ptr percent { juce::ParameterID { percentID, 100 }, "Percent" };
    chowdsp::GainDBParameter::Ptr gain { juce::ParameterID { chowdsp::StringLiteral { "gain" }, 100 }, "Gain", juce::NormalisableRange { -30.0f, 0.0f }, 0.0f };
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

struct PluginNonParameterState : chowdsp::NonParamState
{
    PluginNonParameterState()
    {
        addStateValues ({ &editorBounds });
    }

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

    chowdsp::SharedPluginSettings pluginSettings;
    std::optional<chowdsp::presets::frontend::SettingsInterface> presetsSettings;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StatefulPlugin)
};
