#pragma once

#include "SimpleEQPlugin.h"
#include "FilterPlots.h"

class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (SimpleEQPlugin& plugin);

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    SimpleEQPlugin& plugin;

    juce::GenericAudioProcessorEditor genericEditor;

    FilterPlots plots;

    juce::TextButton saveButton { "Save EQ Params" }, loadButton { "Load EQ Params" };

    std::shared_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
