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
    /** Types of input sources that can be used as presses */
    using PressSourceType = juce::MouseInputSource::InputSourceType;

    /** Parameters for determining whether a long-press has occured */
    struct Parameters
    {
        float dragDistanceThreshold = 8.0f; /** If the mouse has been dragged farther than this threshold, it's not a long-press! */
        int pressLengthMilliseconds = 800; /** The press must be at least this long, otherwise it's not a long-press. */
    };

    /** Default constructor */
    LongPressActionHelper();

    /** Constructor with custom long-press parameters */
    explicit LongPressActionHelper (const Parameters& params);

    /** Default destructor */
    ~LongPressActionHelper() override;

    /** You can assign a lambda to this callback object to have it called when a long-press occurs */
    std::function<void (juce::Point<int> downPosition)> longPressCallback = [] (juce::Point<int>) {};

    /** Sets a component to listen for mouse actions on, or nullptr to not listen for mouse actions */
    void setAssociatedComponent (juce::Component* comp);

    /** Manually start a press at a given position */
    void startPress (const juce::Point<int>& newDownPosition, PressSourceType pressType = PressSourceType::mouse);

    /** Manually abort a press that is in-progress */
    void abortPress();

    /** Returns true if a mouse-press is currently in-progress */
    [[nodiscard]] bool isBeingPressed() const noexcept { return pressStarted; }

    /** Updates how far the mouse has been dragged since the press began */
    void setDragDistance (float newDistance) { dragDistance = newDistance; }

    /** Returns true if long-press actions are enabled for any input source */
    [[nodiscard]] bool isLongPressActionEnabled() const { return ! allowedInputSourceTypes.isEmpty(); }

    /** Returns true if long-press actions are enabled for the given input source */
    [[nodiscard]] bool isLongPressActionEnabled (PressSourceType type) const { return allowedInputSourceTypes.contains (type); }

    /** Use this function to enable/disable long-press actions for various input types */
    void setLongPressSourceTypes (juce::Array<PressSourceType>&& types) { allowedInputSourceTypes = std::move (types); }

    /** Overrides a MouseListener callback */
    void mouseDown (const juce::MouseEvent& e) override;

    /** Overrides a MouseListener callback */
    void mouseDrag (const juce::MouseEvent& e) override;

    /** Overrides a MouseListener callback */
    void mouseUp (const juce::MouseEvent& e) override;

private:
    void timerCallback() override;

    juce::Point<int> downPosition;
    bool pressStarted = false;
    float dragDistance = 8.0f;

    juce::Component* component = nullptr;

    const float dragDistanceThreshold;
    const int pressLengthMs;

    juce::Array<PressSourceType> allowedInputSourceTypes { PressSourceType::touch, PressSourceType::pen };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LongPressActionHelper)
};
} // namespace chowdsp
