#include <DummyPlugin.h>
#include <TimedUnitTest.h>
#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>

class UIStateTest : public TimedUnitTest
{
public:
    UIStateTest() : TimedUnitTest ("UI State Test")
    {
    }

    void uiStateTest()
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

            expectEquals (testComp.getWidth(), defaultWidth, "Default width is not set correctly!");
            expectEquals (testComp.getHeight(), defaultHeight, "Default height is not set correctly!");

            testComp.setSize (testWidth, testHeight);
        }

        {
            juce::GenericAudioProcessorEditor testComp (plugin);
            uiState.attachToComponent (testComp);

            expectEquals (testComp.getWidth(), testWidth, "Restored width is not set correctly!");
            expectEquals (testComp.getHeight(), testHeight, "Restored height is not set correctly!");
        }
    }

    void runTestTimed() override
    {
        beginTest ("UI State Test");
        uiStateTest();
    }
};

static UIStateTest uiStateTest;