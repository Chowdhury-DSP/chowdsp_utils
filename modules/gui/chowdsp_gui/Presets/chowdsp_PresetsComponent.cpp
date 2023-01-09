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
PresetsComponent::PresetsComponent (PresetManager& manager,
                                    PresetsFrontend::FileInterface* fileFace)
    : presetManager (manager),
      prevPresetButton ("", juce::DrawableButton::ImageOnButtonBackground),
      nextPresetButton ("", juce::DrawableButton::ImageOnButtonBackground)
{
    setColour (textColour, juce::Colours::white);
    setColour (backgroundColour, juce::Colours::transparentBlack);
    setColour (bubbleColour, juce::Colours::grey);

    if (fileFace == nullptr)
    {
        fileInterface = OptionalPointer<PresetsFrontend::FileInterface> (presetManager, ".chowpreset");
        fileInterface->savePresetCallback = [this] (nlohmann::json&& presetState)
        {
            saveUserPreset (std::move (presetState));
        };
        fileInterface->checkDeletePresetCallback = [] (const Preset&)
        {
            // @TODO: show warning message!
            return true;
        };
        fileInterface->failedToLoadPresetCallback = [] (const Preset&) {}; // @TODO: show warning message!
    }
    else
    {
        fileInterface.setNonOwning (fileFace);
    }

    menuInterface = std::make_unique<PresetsFrontend::MenuInterface> (presetManager, fileInterface.get());

    presetNameDisplay.setColour (juce::Label::ColourIds::outlineColourId, juce::Colours::transparentBlack);
    presetNameDisplay.setColour (juce::Label::ColourIds::backgroundColourId, juce::Colours::transparentBlack);
    presetNameDisplay.setColour (juce::Label::ColourIds::backgroundWhenEditingColourId, juce::Colours::transparentBlack);
    presetNameDisplay.setColour (juce::Label::ColourIds::outlineColourId, juce::Colours::transparentBlack);
    presetNameDisplay.setInterceptsMouseClicks (false, true);
    presetNameDisplay.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (presetNameDisplay);
    presetNameDisplay.setText (textInterface.getPresetText(), juce::dontSendNotification);
    textInterface.presetTextChangedBroadcaster.connect ([this] (const juce::String& presetText)
                                                        { presetNameDisplay.setText (presetText, juce::sendNotification); });

    presetNameEditor.setColour (juce::TextEditor::ColourIds::outlineColourId, juce::Colours::transparentBlack);
    presetNameEditor.setColour (juce::TextEditor::ColourIds::focusedOutlineColourId, juce::Colours::transparentBlack);
    presetNameEditor.setColour (juce::TextEditor::ColourIds::backgroundColourId, juce::Colours::transparentBlack);
    presetNameEditor.setColour (juce::TextEditor::ColourIds::highlightColourId, juce::Colours::red.withAlpha (0.25f));
    presetNameEditor.setColour (juce::CaretComponent::caretColourId, juce::Colours::red);
    presetNameEditor.setMultiLine (false);
    presetNameEditor.setJustification (juce::Justification::centred);
    addChildComponent (presetNameEditor);
    presetNameEditor.onFocusLost = [this]
    {
        presetNameDisplay.setVisible (true);
        presetNameEditor.setVisible (false);
    };
    presetNameEditor.onEscapeKey = [this]
    {
        presetNameDisplay.setVisible (true);
        presetNameEditor.setVisible (false);
    };

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

void PresetsComponent::paint (juce::Graphics& g)
{
    g.fillAll (findColour (backgroundColour));

    g.setColour (findColour (bubbleColour));
    g.fillRoundedRectangle (presetNameDisplay.getBoundsInParent().toFloat(), 5.0f);
}

void PresetsComponent::resized()
{
    auto b = getLocalBounds();
    prevPresetButton.setBounds (b.removeFromLeft (arrowWidth));
    nextPresetButton.setBounds (b.removeFromRight (arrowWidth));

    presetNameDisplay.setBounds (b.reduced (arrowPad, 0));
    presetNameEditor.setBounds (presetNameDisplay.getBoundsInParent());
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

void PresetsComponent::mouseDown (const juce::MouseEvent&)
{
    if (menuInterface != nullptr)
    {
        juce::PopupMenu menu;
        menuInterface->addPresetsToMenu (menu);

        using Item = PresetsFrontend::MenuInterface::ExtraMenuItems;
        menuInterface->addExtraMenuItems (menu,
                                          { Item::Separator,
                                            Item::Reset,
                                            Item::Save_Preset_As,
                                            Item::Resave_Preset,
                                            Item::Delete_Preset,
                                            Item::Separator,
                                            Item::Copy_Current_Preset,
                                            Item::Paste_Preset,
                                            Item::Load_Preset_From_File,
                                            Item::Separator,
                                            Item::Go_to_User_Presets_Folder,
                                            Item::Choose_User_Presets_Folder });

        menu.showMenuAsync (juce::PopupMenu::Options {}
                                .withTargetComponent (this)
                                .withStandardItemHeight (proportionOfHeight (0.75f)));
    }
}

void PresetsComponent::colourChanged()
{
    presetNameDisplay.setColour (juce::Label::ColourIds::textColourId, findColour (textColour));
    presetNameDisplay.setColour (juce::TextEditor::ColourIds::highlightedTextColourId, findColour (textColour));
}

void PresetsComponent::saveUserPreset (nlohmann::json&& presetState)
{
    presetNameDisplay.setVisible (false);
    presetNameEditor.setVisible (true);

    presetNameEditor.setText ("Preset");
    presetNameEditor.setHighlightedRegion ({ 0, 100 });
    presetNameEditor.grabKeyboardFocus();

    presetNameEditor.onReturnKey = [this, presetState = std::move (presetState)]() mutable
    {
        Preset preset { presetNameEditor.getText(), presetManager.getUserPresetVendorName(), std::move (presetState) };
        const auto file = fileInterface->getFileForPreset (preset);
        // @TODO: warn on file overwrite?

        presetManager.saveUserPreset (file, std::move (preset));
    };
}
} // namespace chowdsp
