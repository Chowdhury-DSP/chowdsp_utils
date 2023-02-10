#include <CatchUtils.h>
#include <chowdsp_gui/chowdsp_gui.h>

TEST_CASE ("Window In Plugin Test", "[gui]")
{
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

    SECTION ("Window Position Test")
    {
        TestComp parentComp;
        chowdsp::WindowInPlugin<TestComp> window (parentComp);

        parentComp.setBounds (0, 0, 200, 200);
        window.show();

        auto parentCentre = parentComp.getBounds().getCentre();
        auto windowCentre = window.getBoundsInParent().getCentre();
        REQUIRE_MESSAGE (parentCentre == windowCentre, "Initial position is incorrect!");

        parentComp.setBounds (0, 0, 300, 300);
        window.show();
        parentCentre = parentComp.getBounds().getCentre();
        windowCentre = window.getBoundsInParent().getCentre();
        REQUIRE_MESSAGE (parentCentre == windowCentre, "Updated position is incorrect!");
    }

    SECTION ("Window Hierarchy Test")
    {
        TestComp parentComp1;
        juce::String testParam { "param" };
        const chowdsp::WindowInPlugin<TestComp> window (parentComp1, testParam);

        REQUIRE_MESSAGE (window.getViewComponent().s == testParam, "Non-default initialisation is incorrect!");

        REQUIRE_MESSAGE (window.getParentComponent() == &parentComp1, "Initial parent is incorrect!");

        TestComp parentComp2;
        parentComp2.addAndMakeVisible (parentComp1);
        REQUIRE_MESSAGE (window.getParentComponent() == &parentComp2, "Updated parent is incorrect!");
    }

    SECTION ("Close Button Test")
    {
        TestComp parentComp;
        chowdsp::WindowInPlugin<TestComp2> window (parentComp, "param2");

        parentComp.setBounds (0, 0, 200, 200);
        window.getViewComponent().setBounds (0, 0, 100, 100);
        REQUIRE_MESSAGE (! window.isVisible(), "Window should not be visible at first!");

        window.show();
        REQUIRE_MESSAGE (window.isVisible(), "Window should now be visible!");

        window.closeButtonPressed();
        REQUIRE_MESSAGE (! window.isVisible(), "Window should not be visible anymore!");
    }
}
