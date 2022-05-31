#include "PluginEditor.h"

PluginEditor::PluginEditor (SimpleEQPlugin& p) : juce::AudioProcessorEditor (p),
                                                 plugin (p),
                                                 genericEditor (plugin)
{
    addAndMakeVisible (genericEditor);
    addAndMakeVisible (saveButton);
    addAndMakeVisible (loadButton);

    setSize (500, 500);
}

void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void PluginEditor::resized()
{
    auto bounds = getLocalBounds();
    genericEditor.setBounds (bounds.removeFromTop (proportionOfHeight (0.93f)));

    const auto buttonWidth = proportionOfWidth (0.33f);
    saveButton.setBounds (bounds.removeFromLeft (buttonWidth));
    loadButton.setBounds (bounds.removeFromLeft (buttonWidth));
}
