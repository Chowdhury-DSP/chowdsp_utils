#include "AccessiblePluginEditor.h"

AccessiblePluginEditor::AccessiblePluginEditor (AccessiblePlugin& p) : juce::AudioProcessorEditor (p),
                                                                       plugin (p)
{
    slider.setSliderStyle (juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    slider.setWantsKeyboardFocus (true);
    tabs.addTab ("Slider", juce::Colours::green, &slider, false);

    menu.addItemList (juce::StringArray { "Item 1", "Item 2", "Item 3" }, 1);
    menu.setSelectedItemIndex (0);
    menu.setDescription ("Test Menu");
    tabs.addTab ("Menu", juce::Colours::blue, &menu, false);

    button.setButtonText ("Demo");
    button.setClickingTogglesState (true);
    tabs.addTab ("Button", juce::Colours::red, &button, false);

    addAndMakeVisible (tabs);

    for (int i = 0; i < tabs.getNumTabs(); ++i)
    {
        auto* tabButton = tabs.getTabbedButtonBar().getTabButton (i);
        tabButton->setWantsKeyboardFocus (true);
    }

    setSize (600, 200);
}

void AccessiblePluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);
}

void AccessiblePluginEditor::resized()
{
    auto bounds = getLocalBounds();
    tabs.setBounds (bounds);
}
