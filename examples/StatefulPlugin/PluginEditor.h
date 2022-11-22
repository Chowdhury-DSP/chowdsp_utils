#pragma once

#include "StatefulPlugin.h"

class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (StatefulPlugin& plug);

    void resized();

private:
    StatefulPlugin& plugin;
    juce::GenericAudioProcessorEditor controls;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
