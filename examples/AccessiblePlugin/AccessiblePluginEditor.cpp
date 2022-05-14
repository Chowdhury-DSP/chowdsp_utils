#include "AccessiblePluginEditor.h"

AccessiblePluginEditor::AccessiblePluginEditor (AccessiblePlugin& p) : juce::AudioProcessorEditor (p),
                                                                       plugin (p)
{
    setSize (400, 400);
}

void AccessiblePluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);
}
