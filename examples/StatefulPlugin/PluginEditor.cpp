#include "PluginEditor.h"
using namespace std::string_view_literals;

PluginEditor::PluginEditor (StatefulPlugin& plug) : juce::AudioProcessorEditor (plug),
                                                    plugin (plug),
                                                    paramsView (plug, plug.getState(), plug.getState().params)
{
    addAndMakeVisible (paramsView);

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
    paramsView.setBounds (getLocalBounds().withHeight (getHeight() - 30));

    undoButton.setBounds (0, getHeight() - 30, 80, 30);
    redoButton.setBounds (80, getHeight() - 30, 80, 30);

    plugin.getState().nonParams.editorBounds = getLocalBounds().getBottomRight();
}
