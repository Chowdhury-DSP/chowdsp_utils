#pragma once

namespace chowdsp
{
/**
 * A class to automatically implement the showing
 * of popup menus, either via right-click on most
 * platforms, of via long-press on iOS.
 */
class PopupMenuHelper : private juce::MouseListener
{
public:
    /** Default constructor */
    PopupMenuHelper();

    /** Constructor with custom long-press parameters */
    explicit PopupMenuHelper (const LongPressActionHelper::Parameters& params);

    /** Default destructor */
    ~PopupMenuHelper() override;

    /** You can assign a lambda to this callback object to fill in the popup menu about to be shown */
    std::function<void (juce::PopupMenu&, juce::PopupMenu::Options&)> popupMenuCallback = [] (juce::PopupMenu&, juce::PopupMenu::Options&) {};

    /** Sets a component to listen for mouse actions on, or nullptr to not listen for mouse actions */
    void setAssociatedComponent (juce::Component* comp);

    /** Use this method to manually trigger the popup menu */
    void showPopupMenu();

    /**
     * Set if long-press actions should trigger popup menus.
     * By default, this flag is true on iOS, and false otherwise.
     */
    void setLongPressEnabled (bool shouldBeEnabled) { longPress.setLongPressActionEnabled (shouldBeEnabled); }

    /** Returns true if long-press actions should trigger popup menus */
    bool isLongPressEnabled() const { return longPress.isLongPressActionEnabled(); }

    /** Overrides a MouseListener callback */
    void mouseDown (const juce::MouseEvent& e) override;

    /** Returns the LongPressActionHelper used to listen for long-presses */
    auto& getLongPressActionHelper() { return longPress; }

private:
    juce::Component* component = nullptr;

    LongPressActionHelper longPress;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PopupMenuHelper)
};
} // namespace chowdsp
