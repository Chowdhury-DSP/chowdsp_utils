#include <CatchUtils.h>
#include <chowdsp_gui/chowdsp_gui.h>

template <typename Callback>
void checkLongPress (bool expectedHit, Callback&& callback, bool shouldBeEnabled = true)
{
    bool hasLongPressOccured = false;
    chowdsp::LongPressActionHelper longPress;
    longPress.setLongPressSourceTypes ({ juce::MouseInputSource::mouse });
    longPress.longPressCallback = [&] (juce::Point<int>)
    { hasLongPressOccured = true; };

    callback (longPress);

    if (expectedHit)
        REQUIRE_MESSAGE (hasLongPressOccured, "Long press did not occur as expected!");
    else
        REQUIRE_MESSAGE (! hasLongPressOccured, "Long press occured when it was not expected!");

    const juce::String enableErrorMessage = "Long-press should " + juce::String (shouldBeEnabled ? "" : "NOT ") + "be enabled!";
    REQUIRE_MESSAGE (shouldBeEnabled == longPress.isLongPressActionEnabled (juce::MouseInputSource::mouse), enableErrorMessage);
}

TEST_CASE ("Long Press Action Test", "[gui]")
{
    auto createDummyMouseEvent = [] (juce::Component* comp, juce::ModifierKeys mods = {})
    {
        auto mouseSource = juce::Desktop::getInstance().getMainMouseSource();
        return juce::MouseEvent { mouseSource, juce::Point<float> {}, mods, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comp, comp, juce::Time::getCurrentTime(), juce::Point<float> {}, juce::Time::getCurrentTime(), 1, false };
    };

    SECTION ("Start and Don't Move Test")
    {
        checkLongPress (true,
                        [] (auto& longPress)
                        {
                            longPress.startPress (juce::Point<int> {});
                            longPress.setDragDistance (1.0f);
                            juce::MessageManager::getInstance()->runDispatchLoopUntil (1000);
                        });
    }

    SECTION ("Start and Move Too Far Test")
    {
        checkLongPress (false,
                        [] (auto& longPress)
                        {
                            longPress.startPress (juce::Point<int> {});
                            longPress.setDragDistance (20.0f);
                            juce::MessageManager::getInstance()->runDispatchLoopUntil (1000);
                        });
    }

    SECTION ("Start and Abort Test")
    {
        checkLongPress (false,
                        [] (auto& longPress)
                        {
                            longPress.startPress (juce::Point<int> {});
                            REQUIRE_MESSAGE (longPress.isBeingPressed(), "Press should be started!");
                            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
                            longPress.abortPress();
                        });
    }

    SECTION ("Long-Press Disabled Test")
    {
        checkLongPress (
            false,
            [] (auto& longPress)
            {
                longPress.setLongPressSourceTypes ({});
                longPress.startPress (juce::Point<int> {});

                REQUIRE_MESSAGE (! longPress.isBeingPressed(), "Press should not be started when long-press is disabled!");

                juce::MessageManager::getInstance()->runDispatchLoopUntil (1000);

                REQUIRE_MESSAGE (! longPress.isLongPressActionEnabled(), "Long-press should be disabled for all sources!");
            },
            false);
    }

    using namespace test_utils;
    SECTION ("Component Long-Press Test")
    {
        checkLongPress (true,
                        [&createDummyMouseEvent] (auto& longPress)
                        {
                            juce::Component comp;
                            longPress.setAssociatedComponent (&comp);

                            longPress.mouseDown (createDummyMouseEvent (&comp));
                            longPress.mouseDrag (createDummyMouseEvent (&comp));
                            juce::MessageManager::getInstance()->runDispatchLoopUntil (1000);
                            longPress.mouseUp (createDummyMouseEvent (&comp));

                            longPress.setAssociatedComponent (nullptr);
                        });
    }

    SECTION ("Component Short-Press Test")
    {
        checkLongPress (false,
                        [&createDummyMouseEvent] (auto& longPress)
                        {
                            juce::Component comp;
                            longPress.setAssociatedComponent (&comp);

                            longPress.mouseDown (createDummyMouseEvent (&comp));
                            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
                            longPress.mouseUp (createDummyMouseEvent (&comp));

                            longPress.setAssociatedComponent (nullptr);
                        });
    }
}
