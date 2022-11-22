#include "PluginEditor.h"

PluginEditor::PluginEditor (StatefulPlugin& plug) : juce::AudioProcessorEditor (plug),
                                                    plugin (plug),
                                                    controls (plugin)
{
    addAndMakeVisible (controls);
    setResizable (true, true);
    setSize (plugin.getState().nonParams.editorWidth, plugin.getState().nonParams.editorHeight);
}

void PluginEditor::resized()
{
    controls.setBounds (getLocalBounds());
    plugin.getState().nonParams.editorWidth = getLocalBounds().getWidth();
    plugin.getState().nonParams.editorHeight = getLocalBounds().getHeight();
}
