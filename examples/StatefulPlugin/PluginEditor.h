#pragma once

#include <chowdsp_gui/chowdsp_gui.h>
#include "StatefulPlugin.h"

class PluginEditor : public juce::AudioProcessorEditor,
                     private juce::ChangeListener
{
public:
    explicit PluginEditor (StatefulPlugin& plug);
    ~PluginEditor() override;

    void resized() override;

private:
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;

    StatefulPlugin& plugin;

    chowdsp::ParametersViewEditor<State, PluginParameterState> paramsView; // { *this, state, state.params };

    void refreshUndoRedoButtons();
    juce::TextButton undoButton { "UNDO" };
    juce::TextButton redoButton { "REDO" };

    chowdsp::ScopedCallbackList editorStateCallbacks;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
