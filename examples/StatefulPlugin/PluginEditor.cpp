#include "PluginEditor.h"

PluginEditor::PluginEditor (StatefulPlugin& plug) : juce::AudioProcessorEditor (plug),
                                                    plugin (plug),
                                                    gainAttach (*plug.getState().params.levelParams.gain,
                                                                plug.getState(),
                                                                gainSlider),
                                                    percentAttach (*plug.getState().params.levelParams.percent,
                                                                   plug.getState(),
                                                                   percentSlider)
{
    const auto addSlider = [this] (juce::Slider& slider)
    {
        addAndMakeVisible (slider);
        slider.setSliderStyle (juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 15);
    };

    addSlider (gainSlider);
    addSlider (percentSlider);

    setResizable (true, true);
    setSize (plugin.getState().nonParams.editorWidth, plugin.getState().nonParams.editorHeight);
}

void PluginEditor::resized()
{
    gainSlider.setBounds (0, 0, 80, 80);
    percentSlider.setBounds (80, 0, 80, 80);

    plugin.getState().nonParams.editorWidth = getLocalBounds().getWidth();
    plugin.getState().nonParams.editorHeight = getLocalBounds().getHeight();
}
