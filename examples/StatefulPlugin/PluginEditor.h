#pragma once

#include "StatefulPlugin.h"

class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (StatefulPlugin& plug);

    void resized() override;

private:
    StatefulPlugin& plugin;

    juce::Slider gainSlider;
    chowdsp::SliderAttachment<State> gainAttach;

    juce::Slider percentSlider;
    chowdsp::SliderAttachment<State> percentAttach;

    juce::ComboBox modeBox;
    chowdsp::ComboBoxAttachment<State> modeAttach;

    juce::ToggleButton onOffButton;
    chowdsp::ButtonAttachment<State> onOffAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
