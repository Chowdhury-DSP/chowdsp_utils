#include "chowdsp_PresetsComponent.h"

namespace
{
#if JUCE_IOS
constexpr int arrowWidth = 32;
constexpr int arrowPad = 4;
#else
constexpr int arrowWidth = 20;
constexpr int arrowPad = 2;
#endif
} // namespace

namespace chowdsp
{
PresetsComponent::PresetsComponent (PresetManager& manager) : presetManager (manager),
                                                              prevPresetButton ("", juce::DrawableButton::ImageOnButtonBackground),
                                                              nextPresetButton ("", juce::DrawableButton::ImageOnButtonBackground)
{
    auto setupNextPrevButton = [this] (juce::DrawableButton& button, bool forward)
    {
        addAndMakeVisible (button);
        button.setWantsKeyboardFocus (false);
        button.setTitle ("Go to " + juce::String (forward ? "next" : "previous") + " preset");
        button.setColour (juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
        button.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        setNextPrevButton (nullptr, forward);
    };

    setupNextPrevButton (prevPresetButton, false);
    prevPresetButton.onClick = [this]
    { presetsNextPrevious.goToPreviousPreset(); };
    setupNextPrevButton (nextPresetButton, true);
    nextPresetButton.onClick = [this]
    { presetsNextPrevious.goToNextPreset(); };
}

void PresetsComponent::paint (juce::Graphics&)
{
    //    g.fillAll (juce::Colours::red);
}

void PresetsComponent::resized()
{
    auto b = getLocalBounds();
    prevPresetButton.setBounds (b.removeFromLeft (arrowWidth));
    nextPresetButton.setBounds (b.removeFromRight (arrowWidth));

    //    juce::Rectangle<int> presetsBound (b.reduced (arrowPad, 0));
    //    presetBox.setBounds (presetsBound);
    //    presetNameEditor.setBounds (presetsBound);
    //    repaint();
}

void PresetsComponent::setNextPrevButton (const juce::Drawable* image, bool isNext)
{
    if (image == nullptr)
    {
        if (isNext)
        {
            auto defaultImage = juce::Drawable::createFromImageData (chowdsp_BinaryData::RightArrow_svg, chowdsp_BinaryData::RightArrow_svgSize);
            nextPresetButton.setImages (defaultImage.get());
        }
        else
        {
            auto defaultImage = juce::Drawable::createFromImageData (chowdsp_BinaryData::LeftArrow_svg, chowdsp_BinaryData::LeftArrow_svgSize);
            prevPresetButton.setImages (defaultImage.get());
        }

        return;
    }

    if (isNext)
        nextPresetButton.setImages (image);
    else
        prevPresetButton.setImages (image);
}
} // namespace chowdsp
