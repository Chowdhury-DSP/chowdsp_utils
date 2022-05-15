#pragma once

#include "AccessiblePlugin.h"
#include "TabbedComponent.h"

class AccessiblePluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit AccessiblePluginEditor (AccessiblePlugin& plugin);

    void resized() override;
    void paint (juce::Graphics& g) override;

private:
    AccessiblePlugin& plugin;

    TabbedComponent tabs { juce::TabbedButtonBar::TabsAtTop };

    juce::Slider slider;
    juce::ComboBox menu;
    juce::TextButton button;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AccessiblePluginEditor)
};
