#pragma once

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

    juce::Slider gainSlider;
    chowdsp::SliderAttachment<State> gainAttach;

    juce::Slider percentSlider;
    chowdsp::SliderAttachment<State> percentAttach;

    juce::ComboBox modeBox;
    chowdsp::ComboBoxAttachment<State> modeAttach;

    juce::ToggleButton onOffButton;
    chowdsp::ButtonAttachment<State> onOffAttach;

    void refreshUndoRedoButtons();
    juce::TextButton undoButton { "UNDO" };
    juce::TextButton redoButton { "REDO" };

    chowdsp::ScopedCallbackList editorStateCallbacks;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
