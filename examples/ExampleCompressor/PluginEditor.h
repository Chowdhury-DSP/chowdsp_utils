#pragma once

#include "ExampleCompressor.h"
#include <chowdsp_gui/chowdsp_gui.h>

class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (ExampleCompressorPlugin& plugin);

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    chowdsp::ComponentArena<> arena {};
    juce::Component* levelDetectorEditor {};
    juce::Component* gainComputerEditor {};
    juce::Component* meter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
