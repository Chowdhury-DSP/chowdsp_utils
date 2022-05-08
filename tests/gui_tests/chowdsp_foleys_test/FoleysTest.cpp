#include <TimedUnitTest.h>
#include <chowdsp_foleys/chowdsp_foleys.h>
#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

/** Empty plugin with Foley's Magic Editor */
class GuiPlugin : public chowdsp::PluginBase<GuiPlugin>
{
public:
    GuiPlugin() = default;

    static void addParameters (Parameters& /*params*/) {}
    void prepareToPlay (double, int) override {}
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>&) override {}

    juce::AudioProcessorEditor* createEditor() override
    {
        auto builder = chowdsp::createGUIBuilder (magicState);
        builder->registerFactory ("PresetsComp", &chowdsp::PresetsItem<GuiPlugin>::factory);
        return new foleys::MagicPluginEditor (magicState, std::move (builder));
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GuiPlugin)
};

/** Unit test to make sure that Foley's Magic Editor can be compiled
 * and created with chowdsp extensions
 */
class FoleysTest : public TimedUnitTest
{
public:
    FoleysTest() : TimedUnitTest ("Foleys GUI Test") {}

    void runTestTimed() override
    {
        beginTest ("Create Foleys Editor Test");
        GuiPlugin guiPlugin;

        std::unique_ptr<juce::AudioProcessorEditor> editor (guiPlugin.createEditor());
        editor->setVisible (true);
    }
};

static FoleysTest foleysTest;
