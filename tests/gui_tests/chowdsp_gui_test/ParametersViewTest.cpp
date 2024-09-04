#include <CatchUtils.h>
#include <chowdsp_gui/chowdsp_gui.h>
#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

namespace params_view_test
{
struct LevelParams : chowdsp::ParamHolder
{
    explicit LevelParams (const juce::String& paramPrefix = "", int version = 100)
        : prefix (paramPrefix),
          versionHint (version)
    {
        add (percent, gain);
    }

    const juce::String prefix;
    const int versionHint;

    chowdsp::PercentParameter::Ptr percent { juce::ParameterID { prefix + "percent", versionHint }, "Percent" };
    chowdsp::GainDBParameter::Ptr gain { juce::ParameterID { prefix + "gain", versionHint }, "Gain", juce::NormalisableRange { -30.0f, 0.0f }, 0.0f };
};

struct PluginParameterState : chowdsp::ParamHolder
{
    PluginParameterState()
    {
        add (levelParams, mode, onOff);
    }

    LevelParams levelParams;
    chowdsp::ChoiceParameter::Ptr mode { "mode", "Mode", juce::StringArray { "Percent", "Gain", "Percent / Gain", "Gain / Percent" }, 2 };
    chowdsp::BoolParameter::Ptr onOff { "on_off", "On/Off", true };
};

using State = chowdsp::PluginStateImpl<PluginParameterState>;

struct Plugin : chowdsp::PluginBase<State>
{
    const juce::String getName() const override { return "Plugin"; }
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override { buffer.clear(); }
    juce::AudioProcessorEditor* createEditor() override { return new chowdsp::ParametersViewEditor { *this }; }
};
} // namespace params_view_test

TEST_CASE ("Parameters View Test", "[gui][state]")
{
    params_view_test::Plugin plugin;

    auto* editor = dynamic_cast<chowdsp::ParametersViewEditor*> (plugin.createEditor());
    REQUIRE (editor != nullptr);
    auto _ = chowdsp::runAtEndOfScope (
        [&plugin, editor]
        {
            plugin.editorBeingDeleted (editor);
            delete editor;
        });

    editor->setSize (600, 600);

    auto& params = plugin.getState().params;
    REQUIRE (editor->view.getComponentForParameter (params.onOff) != nullptr);
    REQUIRE (editor->view.getComponentForParameter (params.mode) != nullptr);
    REQUIRE (editor->view.getComponentForParameter (params.levelParams.gain) != nullptr);
    REQUIRE (editor->view.getComponentForParameter (params.levelParams.percent) != nullptr);
}
