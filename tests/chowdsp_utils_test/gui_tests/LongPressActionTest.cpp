#include "TimedUnitTest.h"
#include "test_utils.h"

class LongPressActionTest : public TimedUnitTest
{
public:
    LongPressActionTest() : TimedUnitTest ("Long Press Action Test")
    {
    }

    template <typename Callback>
    void checkLongPress (bool expectedHit, Callback&& callback, bool shouldBeEnabled = true)
    {
        bool hasLongPressOccured = false;
        chowdsp::LongPressActionHelper longPress;
        longPress.setLongPressSourceTypes ({ MouseInputSource::mouse });
        longPress.longPressCallback = [&] (Point<int>)
        { hasLongPressOccured = true; };

        callback (longPress);

        if (expectedHit)
            expect (hasLongPressOccured, "Long press did not occur as expected!");
        else
            expect (! hasLongPressOccured, "Long press occured when it was not expected!");

        const String enableErrorMessage = "Long-press should " + String (shouldBeEnabled ? "" : "NOT ") + "be enabled!";
        expect (shouldBeEnabled == longPress.isLongPressActionEnabled (MouseInputSource::mouse), enableErrorMessage);
    }

    void runTestTimed() override
    {
        beginTest ("Start and Don't Move Test");
        checkLongPress (true,
                        [] (auto& longPress)
                        {
                            longPress.startPress (Point<int> {});
                            longPress.setDragDistance (1.0f);
                            MessageManager::getInstance()->runDispatchLoopUntil (1000);
                        });

        beginTest ("Start and Move Too Far Test");
        checkLongPress (false,
                        [] (auto& longPress)
                        {
                            longPress.startPress (Point<int> {});
                            longPress.setDragDistance (20.0f);
                            MessageManager::getInstance()->runDispatchLoopUntil (1000);
                        });

        beginTest ("Start and Abort Test");
        checkLongPress (false,
                        [this] (auto& longPress)
                        {
                            longPress.startPress (Point<int> {});
                            expect (longPress.isBeingPressed(), "Press should be started!");
                            MessageManager::getInstance()->runDispatchLoopUntil (200);
                            longPress.abortPress();
                        });

        beginTest ("Long-Press Disabled Test");
        checkLongPress (
            false,
            [this] (auto& longPress)
            {
                longPress.setLongPressSourceTypes ({});
                longPress.startPress (Point<int> {});

                expect (! longPress.isBeingPressed(), "Press should not be started when long-press is disabled!");

                MessageManager::getInstance()->runDispatchLoopUntil (1000);

                expect (! longPress.isLongPressActionEnabled(), "Long-press should be disabled for all sources!");
            },
            false);

        using namespace test_utils;
        beginTest ("Component Long-Press Test");
        checkLongPress (true,
                        [=] (auto& longPress)
                        {
                            Component comp;
                            longPress.setAssociatedComponent (&comp);

                            longPress.mouseDown (createDummyMouseEvent (&comp));
                            longPress.mouseDrag (createDummyMouseEvent (&comp));
                            MessageManager::getInstance()->runDispatchLoopUntil (1000);
                            longPress.mouseUp (createDummyMouseEvent (&comp));

                            longPress.setAssociatedComponent (nullptr);
                        });

        beginTest ("Component Short-Press Test");
        checkLongPress (false,
                        [=] (auto& longPress)
                        {
                            Component comp;
                            longPress.setAssociatedComponent (&comp);

                            longPress.mouseDown (createDummyMouseEvent (&comp));
                            MessageManager::getInstance()->runDispatchLoopUntil (200);
                            longPress.mouseUp (createDummyMouseEvent (&comp));

                            longPress.setAssociatedComponent (nullptr);
                        });
    }
};

static LongPressActionTest longPressActionTest;
