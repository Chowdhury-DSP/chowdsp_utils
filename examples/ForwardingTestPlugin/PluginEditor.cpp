#include "PluginEditor.h"

PluginEditor::PluginEditor (ForwardingTestPlugin& p) : juce::AudioProcessorEditor (p),
                                                       plugin (p),
                                                       mainEditor (plugin)
{
    addAndMakeVisible (mainEditor);

    setSize (800, 400);

    processorChangedCallback = plugin.getState().addParameterListener (
        plugin.getState().params.processorChoice,
        chowdsp::ParameterListenerThread::MessageThread,
        [this]
        {
            updateProcessorEditor (plugin.getState().params.processorChoice->getIndex());
        });

    updateProcessorEditor (plugin.getState().params.processorChoice->getIndex());
}

PluginEditor::~PluginEditor() = default;

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
