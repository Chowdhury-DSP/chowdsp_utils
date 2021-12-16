#pragma once

namespace chowdsp
{

/**
 * JUCE currently does not have a way of doing
 * "long-press" mouse actions that are pretty convenient
 * on touch screens. This helper class can be used
 * to implement long-press actions for a component.
 */
class LongPressActionHelper : private juce::Timer,
                              private juce::MouseListener
{
public:
    struct Parameters
    {
        float dragDistanceThreshold = 8.0f;
        int pressLengthMilliseconds = 800;
    };

    explicit LongPressActionHelper (const Parameters& params);
    ~LongPressActionHelper() override;

    std::function<void (juce::Point<int> downPosition)> longPressCallback = [] (juce::Point<int>) {};

    void setAssociatedComponent (juce::Component* comp);

    void startPress (const juce::Point<int>& newDownPosition);
    void abortPress();

    bool isBeingPressed() const noexcept { return pressStarted; }
    void setDragDistance (float newDistance) { dragDistance = newDistance; }

    bool isLongPressActionEnabled() const { return isEnabled; }
    void setLongPressActionEnabled (bool shouldBeEnabled) { isEnabled = shouldBeEnabled; }

private:
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;

    void timerCallback() override;

    juce::Point<int> downPosition;
    bool pressStarted = false;
    float dragDistance = 8.0f;

    juce::Component* component = nullptr;

    const float dragDistanceThreshold;
    const int pressLengthMs;

    bool isEnabled = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LongPressActionHelper)
};
} // namespace chowdsp
