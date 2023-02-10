#include <CatchUtils.h>
#include <chowdsp_gui/chowdsp_gui.h>
#include <chowdsp_parameters/chowdsp_parameters.h>
#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

struct PluginParameterState : chowdsp::ParamHolder
{
    PluginParameterState()
    {
        add (onOff);
    }
    chowdsp::BoolParameter::Ptr onOff { "on_off", "On/Off", true };
};

struct DummyPlugin : chowdsp::PluginBase<chowdsp::PluginStateImpl<PluginParameterState>>
{
    DummyPlugin() = default;
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>&) override {}
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }

    struct Editor : juce::AudioProcessorEditor
    {
        explicit Editor (DummyPlugin& plugin) : juce::AudioProcessorEditor (plugin),
                                                hcp (plugin, *this)
        {
            setSize (100, 100);
        }

        void paint (juce::Graphics& g) override
        {
            g.fillAll (juce::Colours::black);
        }

        chowdsp::HostContextProvider hcp;

        std::unique_ptr<juce::Component> paramComponent;
    } editor { *this };
};

TEST_CASE ("Host Context Provider Test", "[gui][parameters]")
{
    SECTION ("Get Context Menu For Parameter Test")
    {
        juce::AudioProcessor::setTypeOfNextNewPlugin (juce::AudioProcessor::wrapperType_VST3);
        DummyPlugin plugin {};
        auto& editor = plugin.editor;

        auto&& menu = editor.hcp.getContextMenuForParameter (*plugin.getState().params.onOff);
        REQUIRE (menu == nullptr); // since we're not in a host, we won't get a context menu back :(

        juce::AudioProcessor::setTypeOfNextNewPlugin (juce::AudioProcessor::wrapperType_Undefined);
    }

    SECTION ("Get Parameter Index For Component Test")
    {
        DummyPlugin plugin {};
        auto& editor = plugin.editor;

        editor.paramComponent = std::make_unique<juce::Component>();
        editor.hcp.registerParameterComponent (*editor.paramComponent, *plugin.getState().params.onOff);
        REQUIRE_MESSAGE (editor.hcp.getParameterIndexForComponent (*editor.paramComponent) == 0, "Incorrect param index for component!");
        editor.paramComponent.reset();

        editor.paramComponent = std::make_unique<juce::Component>();
        REQUIRE_MESSAGE (editor.hcp.getParameterIndexForComponent (*editor.paramComponent) == -1, "Incorrect param index for unregistered component!");
    }
}
