#include "PluginEditor.h"
using namespace std::string_view_literals;

PluginEditor::PluginEditor (StatefulPlugin& plug) : juce::AudioProcessorEditor (plug),
                                                    plugin (plug),
                                                    gainAttach ("LevelParams/gain"sv,
                                                                plug.getState(),
                                                                gainSlider),
                                                    percentAttach ("LevelParams/percent"sv,
                                                                   plug.getState(),
                                                                   percentSlider),
                                                    modeAttach ("mode"sv,
                                                                plug.getState(),
                                                                modeBox),
                                                    onOffAttach ("on_off"sv,
                                                                 plug.getState(),
                                                                 onOffButton)
{
    const auto addSlider = [this] (juce::Slider& slider) {
        addAndMakeVisible (slider);
        slider.setSliderStyle (juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 15);
    };

    addSlider (gainSlider);
    addSlider (percentSlider);

    addAndMakeVisible (modeBox);
    addAndMakeVisible (onOffButton);

    addAndMakeVisible (undoButton);
    undoButton.onClick = [this] {
        plugin.undoManager.undo();
    };

    addAndMakeVisible (redoButton);
    redoButton.onClick = [this] {
        plugin.undoManager.redo();
    };

    plugin.undoManager.addChangeListener (this);
    refreshUndoRedoButtons();

    setResizable (true, true);

    const auto setSizeFromState = [this] {
        const auto& stateSize = plugin.getState().nonParams.editorBounds.get();
        setSize (stateSize.x, stateSize.y);
    };
    setSizeFromState();

    editorStateCallbacks += {
        plugin.getState().addNonParameterListener (plugin.getState().nonParams.editorBounds, [setSizeFromState] { setSizeFromState(); }),
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
    gainSlider.setBounds (0, 0, 80, 80);
    percentSlider.setBounds (80, 0, 80, 80);

    modeBox.setBounds (0, 100, 120, 30);
    onOffButton.setBounds (0, 150, 80, 30);

    undoButton.setBounds (0, getHeight() - 30, 80, 30);
    redoButton.setBounds (80, getHeight() - 30, 80, 30);

    plugin.getState().nonParams.editorBounds = getLocalBounds().getBottomRight();
}
