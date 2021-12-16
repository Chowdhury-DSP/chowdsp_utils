#include "TimedUnitTest.h"

class LongPressActionTest : public TimedUnitTest
{
public:
    LongPressActionTest() : TimedUnitTest ("Long Press Action Test")
    {
    }

    template <typename Callback>
    void checkLongPress (bool expectedHit, Callback&& callback)
    {
        bool hasLongPressOccured = false;
        chowdsp::LongPressActionHelper longPress;
        longPress.longPressCallback = [&] (Point<int>)
        { hasLongPressOccured = true; };

        callback (longPress);

        if (expectedHit)
            expect (hasLongPressOccured, "Long press did not occur as expected!");
        else
            expect (! hasLongPressOccured, "Long press occured when it was not expected!");
    }

    auto createDummyMouseEvent (Component* comp)
    {
        auto mouseSource = Desktop::getInstance().getMainMouseSource();
        return MouseEvent { mouseSource, Point<float> {}, ModifierKeys {}, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comp, comp, Time::getCurrentTime(), Point<float> {}, Time::getCurrentTime(), 1, false };
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
        checkLongPress (false,
                        [this] (auto& longPress)
                        {
                            longPress.setLongPressActionEnabled (false);
                            longPress.startPress (Point<int> {});

                            expect (! longPress.isBeingPressed(), "Press should not be started when long-press is disabled!");

                            MessageManager::getInstance()->runDispatchLoopUntil (1000);

                            expect (! longPress.isLongPressActionEnabled(), "Long-press should be disabled!");
                        });

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
                        });
    }
};

static LongPressActionTest longPressActionTest;
