#include "chowdsp_LongPressActionHelper.h"

namespace chowdsp
{
LongPressActionHelper::LongPressActionHelper() : LongPressActionHelper (Parameters())
{
}

LongPressActionHelper::LongPressActionHelper (const Parameters& params) : dragDistanceThreshold (params.dragDistanceThreshold),
                                                                          pressLengthMs (params.pressLengthMilliseconds)
{
}

LongPressActionHelper::~LongPressActionHelper()
{
    stopTimer();

    setAssociatedComponent (nullptr);
}

void LongPressActionHelper::setAssociatedComponent (juce::Component* comp)
{
    if (component != nullptr)
        component->removeMouseListener (this);

    component = comp;
    if (component != nullptr)
        component->addMouseListener (this, false);
}

void LongPressActionHelper::startPress (const juce::Point<int>& newDownPosition, PressSourceType pressType)
{
    if (! allowedInputSourceTypes.contains (pressType))
        return;

    pressStarted = true;
    downPosition = newDownPosition;
    startTimer (pressLengthMs);
}

void LongPressActionHelper::abortPress()
{
    stopTimer();
    pressStarted = false;
    dragDistance = 0.0f;
}

void LongPressActionHelper::mouseDown (const juce::MouseEvent& e)
{
    startPress (e.getMouseDownPosition(), e.source.getType());
}

void LongPressActionHelper::mouseDrag (const juce::MouseEvent& e)
{
    setDragDistance ((float) e.getDistanceFromDragStart());
}

void LongPressActionHelper::mouseUp (const juce::MouseEvent&)
{
    abortPress();
}

void LongPressActionHelper::timerCallback()
{
    bool wasPressStarted = pressStarted;
    bool wasDragged = dragDistance > dragDistanceThreshold;
    abortPress();

    if (! wasPressStarted)
    {
        // No press started? That's not good.
        jassertfalse;
        return;
    }

    if (wasDragged)
        return;

    longPressCallback (downPosition);
}

} // namespace chowdsp
