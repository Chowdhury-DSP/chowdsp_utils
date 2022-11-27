#pragma once

#include "StatefulPlugin.h"

class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (StatefulPlugin& plug);

    void resized();

private:
    StatefulPlugin& plugin;

    juce::Slider gainSlider;
    chowdsp::SliderAttachment<State> gainAttach;

    juce::Slider percentSlider;
    chowdsp::SliderAttachment<State> percentAttach;

//    juce::GenericAudioProcessorEditor controls;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
