#include <CatchUtils.h>
//#include "test_utils.h"
#include <chowdsp_gui/chowdsp_gui.h>

template <typename Callback>
void checkPopupMenu (bool expectedHit, Callback&& callback)
{
    bool hasPopupMenuShown = false;

    juce::Component comp;
    chowdsp::PopupMenuHelper popupMenu;
    popupMenu.popupMenuCallback = [&] (juce::PopupMenu&, juce::PopupMenu::Options&, juce::Point<int>)
    { hasPopupMenuShown = true; };
    popupMenu.setAssociatedComponent (&comp);

    callback (popupMenu, comp);

    if (expectedHit)
        REQUIRE_MESSAGE (hasPopupMenuShown, "PopupMenu was not shown as expected!");
    else
        REQUIRE_MESSAGE (! hasPopupMenuShown, "PopupMenu was shown when it was not expected!");
}

TEST_CASE ("Popup Menu Helper Test", "[gui]")
{
    auto createDummyMouseEvent = [] (juce::Component* comp, juce::ModifierKeys mods = {})
    {
        auto mouseSource = juce::Desktop::getInstance().getMainMouseSource();
        return juce::MouseEvent { mouseSource, juce::Point<float> {}, mods, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comp, comp, juce::Time::getCurrentTime(), juce::Point<float> {}, juce::Time::getCurrentTime(), 1, false };
    };

    SECTION ("Normal Mouse-Click Test")
    {
        checkPopupMenu (false,
                        [&] (auto& menuHelper, auto& comp)
                        {
                            menuHelper.mouseDown (createDummyMouseEvent (&comp));
                        });
    }

    SECTION ("RMB Mouse-Click Test")
    {
        checkPopupMenu (true,
                        [&] (auto& menuHelper, auto& comp)
                        {
                            juce::ModifierKeys mods { juce::ModifierKeys::Flags::rightButtonModifier };
                            auto mouseEvent = createDummyMouseEvent (&comp, mods);
                            menuHelper.mouseDown (mouseEvent);
                        });
    }

    SECTION ("Long-Press Test")
    {
        checkPopupMenu (true,
                        [&createDummyMouseEvent] (auto& menuHelper, auto& comp)
                        {
                            menuHelper.setLongPressSourceTypes ({ juce::MouseInputSource::mouse });
                            REQUIRE_MESSAGE (menuHelper.isLongPressEnabled(), "Long-presses should be enabled!");

                            auto& longPress = menuHelper.getLongPressActionHelper();
                            longPress.mouseDown (createDummyMouseEvent (&comp));

                            juce::MessageManager::getInstance()->runDispatchLoopUntil (1000);
                        });
    }

    SECTION ("Short-Press Test")
    {
        checkPopupMenu (false,
                        [&createDummyMouseEvent] (auto& menuHelper, auto& comp)
                        {
                            menuHelper.setLongPressSourceTypes ({ juce::MouseInputSource::mouse });
                            REQUIRE_MESSAGE (menuHelper.isLongPressEnabled (juce::MouseInputSource::mouse), "Long-presses should be enabled for mouse source type!");

                            auto& longPress = menuHelper.getLongPressActionHelper();
                            longPress.mouseDown (createDummyMouseEvent (&comp));

                            juce::MessageManager::getInstance()->runDispatchLoopUntil (200);
                        });
    }
}
