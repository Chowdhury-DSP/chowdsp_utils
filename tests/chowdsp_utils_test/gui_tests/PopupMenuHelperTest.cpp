#include "TimedUnitTest.h"
#include "test_utils.h"

class PopupMenuHelperTest : public TimedUnitTest
{
public:
    PopupMenuHelperTest() : TimedUnitTest ("Popup Menu Helper Test")
    {
    }

    template <typename Callback>
    void checkPopupMenu (bool expectedHit, Callback&& callback)
    {
        bool hasPopupMenuShown = false;

        Component comp;
        chowdsp::PopupMenuHelper popupMenu;
        popupMenu.popupMenuCallback = [&] (PopupMenu&, PopupMenu::Options&) { hasPopupMenuShown = true; };
        popupMenu.setAssociatedComponent (&comp);

        callback (popupMenu, comp);

        if (expectedHit)
            expect (hasPopupMenuShown, "PopupMenu was not shown as expected!");
        else
            expect (! hasPopupMenuShown, "PopupMenu was shown when it was not expected!");
    }

    void runTestTimed() override
    {
        using namespace test_utils;

        beginTest ("Normal Mouse-Click Test");
        checkPopupMenu (false,
                        [&] (auto& menuHelper, auto& comp) {
                            menuHelper.mouseDown (createDummyMouseEvent (&comp));
                        });

        beginTest ("RMB Mouse-Click Test");
        checkPopupMenu (true,
                        [&] (auto& menuHelper, auto& comp) {
                            ModifierKeys mods { ModifierKeys::Flags::rightButtonModifier };
                            auto mouseEvent = createDummyMouseEvent (&comp, mods);
                            menuHelper.mouseDown (mouseEvent);
                        });

        beginTest ("Long-Press Test");
        checkPopupMenu (true,
                        [&] (auto& menuHelper, auto& comp) {
                            menuHelper.setLongPressSourceTypes ({ MouseInputSource::mouse });
                            expect (menuHelper.isLongPressEnabled(), "Long-presses should be enabled!");

                            auto& longPress = menuHelper.getLongPressActionHelper();
                            longPress.mouseDown (createDummyMouseEvent (&comp));

                            MessageManager::getInstance()->runDispatchLoopUntil (1000);
                        });

        beginTest ("Short-Press Test");
        checkPopupMenu (false,
                        [&] (auto& menuHelper, auto& comp) {
                            menuHelper.setLongPressSourceTypes ({ MouseInputSource::mouse });
                            expect (menuHelper.isLongPressEnabled (MouseInputSource::mouse), "Long-presses should be enabled for mouse source type!");

                            auto& longPress = menuHelper.getLongPressActionHelper();
                            longPress.mouseDown (createDummyMouseEvent (&comp));

                            MessageManager::getInstance()->runDispatchLoopUntil (200);
                        });
    }
};

static PopupMenuHelperTest popupMenuHelperTest;
