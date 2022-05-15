#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class TabbedComponent : public juce::TabbedComponent
{
public:
    explicit TabbedComponent (juce::TabbedButtonBar::Orientation buttonOrientation) : juce::TabbedComponent (buttonOrientation) {}

protected:
    juce::TabBarButton* createTabButton (const juce::String& tabName, int tabIndex) override
    {
        auto* newButton = juce::TabbedComponent::createTabButton (tabName, tabIndex);
        newButton->setAccessible (true);
        newButton->setWantsKeyboardFocus (true);
        newButton->setDescription ("Tab: " + tabName);

        return newButton;
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TabbedComponent)
};
