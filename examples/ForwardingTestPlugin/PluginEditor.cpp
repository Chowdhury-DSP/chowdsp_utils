#include "PluginEditor.h"

PluginEditor::PluginEditor (ForwardingTestPlugin& p) : juce::AudioProcessorEditor (p),
                                                       plugin (p),
                                                       mainEditor (plugin)
{
    addAndMakeVisible (mainEditor);

    setSize (800, 400);

    plugin.getVTS().addParameterListener (ForwardingTestPlugin::processorChoiceParamID, this);
    updateProcessorEditor ((int) plugin.getVTS().getRawParameterValue (ForwardingTestPlugin::processorChoiceParamID)->load());
}

PluginEditor::~PluginEditor()
{
    plugin.getVTS().removeParameterListener (ForwardingTestPlugin::processorChoiceParamID, this);
}

void PluginEditor::parameterChanged (const juce::String& parameterID, float newValue)
{
    if (parameterID != ForwardingTestPlugin::processorChoiceParamID)
    {
        jassertfalse;
        return;
    }

    updateProcessorEditor ((int) newValue);
}

void PluginEditor::updateProcessorEditor (int processorIndex)
{
    if (auto* newProcessor = plugin.getProcessorForIndex (processorIndex))
    {
        processorEditor = std::make_unique<juce::GenericAudioProcessorEditor> (*newProcessor);
        addAndMakeVisible (processorEditor.get());
        resized();
    }
    else
    {
        processorEditor.reset();
    }
}

void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void PluginEditor::resized()
{
    auto bounds = getLocalBounds();
    mainEditor.setBounds (bounds.removeFromLeft (proportionOfWidth (0.5f)));

    if (processorEditor != nullptr)
        processorEditor->setBounds (bounds);
}
