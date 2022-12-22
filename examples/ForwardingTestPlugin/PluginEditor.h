#pragma once

#include "ForwardingTestPlugin.h"

class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (ForwardingTestPlugin& plugin);
    ~PluginEditor() override;

    void resized() override;
    void paint (juce::Graphics& g) override;

private:
    void updateProcessorEditor (int processorIndex);

    ForwardingTestPlugin& plugin;

    juce::GenericAudioProcessorEditor mainEditor;
    std::unique_ptr<juce::GenericAudioProcessorEditor> processorEditor;

    chowdsp::ScopedCallback processorChangedCallback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
