#pragma once

#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

struct LevelParams
{
    static constexpr std::string_view name = "level_params";
    chowdsp::PercentParameter::Ptr percent { "percent", "Percent" };
    chowdsp::GainDBParameter::Ptr gain { "gain", "Gain", juce::NormalisableRange { -30.0f, 0.0f }, 0.0f };
};

struct PluginParameterState
{
    LevelParams levelParams;
    chowdsp::ChoiceParameter::Ptr mode { "mode", "Mode", juce::StringArray { "Percent", "Gain", "Percent / Gain", "Gain / Percent" }, 2 };
    chowdsp::BoolParameter::Ptr onOff { "on_off", "On/Off", true };
};

struct PluginNonParameterState
{
    int editorWidth = 300;
    int editorHeight = 500;
};

static_assert (chowdsp::plugin_state_detail::ParamCount<PluginParameterState> == 4);
static_assert (chowdsp::plugin_state_detail::ParamCount<LevelParams> == 2);

using State = chowdsp::PluginState<PluginParameterState, PluginNonParameterState>;

class StatefulPlugin : public chowdsp::PluginBase<State>
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
