#include <TimedUnitTest.h>
#include <chowdsp_gui/chowdsp_gui.h>

class WindowInPluginTest : public TimedUnitTest
{
public:
    WindowInPluginTest() : TimedUnitTest ("Window In Plugin Test") {}

    struct TestComp : public juce::Component
    {
        juce::String s;
        explicit TestComp (const juce::String& param = {}) : s (param)
        {
            setSize (100, 100);
        }
    };

    struct TestComp2 : public juce::Component
    {
        juce::String s;
        explicit TestComp2 (const juce::String& param) : s (param)
        {
            setSize (100, 100);
        }
    };

    void windowPositionTest()
    {
        TestComp parentComp;
        chowdsp::WindowInPlugin<TestComp> window (parentComp);

        parentComp.setBounds (0, 0, 200, 200);
        window.show();

        auto parentCentre = parentComp.getBounds().getCentre();
        auto windowCentre = window.getBoundsInParent().getCentre();
        expect (parentCentre == windowCentre, "Initial position is incorrect!");

        parentComp.setBounds (0, 0, 300, 300);
        window.show();
        parentCentre = parentComp.getBounds().getCentre();
        windowCentre = window.getBoundsInParent().getCentre();
        expect (parentCentre == windowCentre, "Updated position is incorrect!");
    }

    void windowHierarchyTest()
    {
        TestComp parentComp1;
        juce::String testParam { "param" };
        const chowdsp::WindowInPlugin<TestComp> window (parentComp1, testParam);

        expectEquals (window.getViewComponent().s, testParam, "Non-default initialisation is incorrect!");

        expect (window.getParentComponent() == &parentComp1, "Initial parent is incorrect!");

        TestComp parentComp2;
        parentComp2.addAndMakeVisible (parentComp1);
        expect (window.getParentComponent() == &parentComp2, "Updated parent is incorrect!");
    }

    void closebuttonTest()
    {
        TestComp parentComp;
        chowdsp::WindowInPlugin<TestComp2> window (parentComp, "param2");

        parentComp.setBounds (0, 0, 200, 200);
        window.getViewComponent().setBounds (0, 0, 100, 100);
        expect (! window.isVisible(), "Window should not be visible at first!");

        window.show();
        expect (window.isVisible(), "Window should now be visible!");

        window.closeButtonPressed();
        expect (! window.isVisible(), "Window should not be visible anymore!");
    }

    void runTestTimed() override
    {
        beginTest ("Window Position Test");
        windowPositionTest();

        beginTest ("Window Hierarchy Test");
        windowHierarchyTest();

        beginTest ("Close Button Test");
        closebuttonTest();
    }
};

static WindowInPluginTest windowInPluginTest;
