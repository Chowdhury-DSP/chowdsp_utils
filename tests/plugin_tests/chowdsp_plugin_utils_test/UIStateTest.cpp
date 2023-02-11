#include <CatchUtils.h>
#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>

#if ! JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
#include <DummyPlugin.h>

TEST_CASE ("UI State Test", "[plugin][utilities]")
{
    SECTION ("UI State Test")
    {
        static constexpr int defaultWidth = 200;
        static constexpr int defaultHeight = 300;
        static constexpr int testWidth = 500;
        static constexpr int testHeight = 600;

        DummyPlugin plugin;
        chowdsp::UIState uiState (plugin.getVTS(), defaultWidth, defaultHeight);

        {
            juce::GenericAudioProcessorEditor testComp (plugin);
            uiState.attachToComponent (testComp);

            REQUIRE_MESSAGE (testComp.getWidth() == defaultWidth, "Default width is not set correctly!");
            REQUIRE_MESSAGE (testComp.getHeight() == defaultHeight, "Default height is not set correctly!");

            testComp.setSize (testWidth, testHeight);
        }

        {
            juce::GenericAudioProcessorEditor testComp (plugin);
            uiState.attachToComponent (testComp);

            REQUIRE_MESSAGE (testComp.getWidth() == testWidth, "Restored width is not set correctly!");
            REQUIRE_MESSAGE (testComp.getHeight() == testHeight, "Restored height is not set correctly!");
        }
    }
}
#endif
