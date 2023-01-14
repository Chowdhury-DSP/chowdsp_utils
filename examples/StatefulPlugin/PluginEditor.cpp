#include "PluginEditor.h"
using namespace std::string_view_literals;

PluginEditor::PluginEditor (StatefulPlugin& plug) : juce::AudioProcessorEditor (plug),
                                                    plugin (plug),
                                                    paramsView (plug, plug.getState(), plug.getState().params)
{
    addAndMakeVisible (paramsView);
    addAndMakeVisible (presetsComp);

    addAndMakeVisible (undoButton);
    undoButton.onClick = [this]
    {
        plugin.undoManager.undo();
    };

    addAndMakeVisible (redoButton);
    redoButton.onClick = [this]
    {
        plugin.undoManager.redo();
    };

    plugin.undoManager.addChangeListener (this);
    refreshUndoRedoButtons();

    setResizable (true, true);

    const auto setSizeFromState = [this]
    {
        const auto& stateSize = plugin.getState().nonParams.editorBounds.get();
        setSize (stateSize.x, stateSize.y);
    };
    setSizeFromState();

    editorStateCallbacks += {
        plugin.getState().addNonParameterListener (plugin.getState().nonParams.editorBounds, [setSizeFromState]
                                                   { setSizeFromState(); }),
    };
}

PluginEditor::~PluginEditor()
{
    plugin.undoManager.removeChangeListener (this);
}

void PluginEditor::changeListenerCallback (juce::ChangeBroadcaster* source)
{
    if (source != &plugin.undoManager)
        return;

    refreshUndoRedoButtons();
}

void PluginEditor::refreshUndoRedoButtons()
{
    undoButton.setEnabled (plugin.undoManager.canUndo());
    redoButton.setEnabled (plugin.undoManager.canRedo());
}

void PluginEditor::resized()
{
    auto bounds = getLocalBounds();
    paramsView.setBounds (bounds.removeFromTop (getHeight() - 70));
    presetsComp.setBounds (bounds.removeFromTop (40).removeFromLeft (180));
    undoButton.setBounds (bounds.removeFromLeft (80));
    redoButton.setBounds (bounds.removeFromLeft (80));

    plugin.getState().nonParams.editorBounds = getLocalBounds().getBottomRight();
}
