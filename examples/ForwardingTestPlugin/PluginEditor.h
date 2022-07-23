#pragma once

#include "ForwardingTestPlugin.h"

class PluginEditor : public juce::AudioProcessorEditor,
                     private juce::AudioProcessorValueTreeState::Listener
{
public:
    explicit PluginEditor (ForwardingTestPlugin& plugin);
    ~PluginEditor() override;

    void resized() override;
    void paint (juce::Graphics& g) override;

private:
    void updateProcessorEditor (int processorIndex);
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    ForwardingTestPlugin& plugin;

    juce::GenericAudioProcessorEditor mainEditor;
    std::unique_ptr<juce::GenericAudioProcessorEditor> processorEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
