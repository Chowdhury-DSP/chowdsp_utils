#pragma once

#include "AccessiblePlugin.h"

class AccessiblePluginEditor : public juce::AudioProcessorEditor
{
public:
    AccessiblePluginEditor (AccessiblePlugin& plugin);

    void paint (juce::Graphics& g) override;

private:
    AccessiblePlugin& plugin;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AccessiblePluginEditor)
};
