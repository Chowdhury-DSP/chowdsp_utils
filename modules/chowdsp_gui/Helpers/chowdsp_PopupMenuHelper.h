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
    PopupMenuHelper();
    explicit PopupMenuHelper (const LongPressActionHelper::Parameters& params);
    ~PopupMenuHelper() override;

    std::function<void (juce::PopupMenu&, juce::PopupMenu::Options&)> popupMenuCallback = [] (juce::PopupMenu&, juce::PopupMenu::Options&) {};

    void setAssociatedComponent (juce::Component* comp);

    void showPopupMenu();

    void setLongPressEnabled (bool shouldBeEnabled) { longPress.setLongPressActionEnabled (shouldBeEnabled); }
    bool isLongPressEnabled() const { return longPress.isLongPressActionEnabled(); }

private:
    void mouseDown (const juce::MouseEvent& e) override;

    juce::Component* component = nullptr;

    LongPressActionHelper longPress;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PopupMenuHelper)
};
} // namespace chowdsp
