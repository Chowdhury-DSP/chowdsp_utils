namespace
{
#if JUCE_IOS
constexpr int arrowWidth = 32;
constexpr int arrowPad = 4;
#else
constexpr int arrowWidth = 20;
constexpr int arrowPad = 2;
#endif

const juce::String presetExt = ".chowpreset";
} // namespace

namespace chowdsp
{
PresetsComp::PresetsComp (PresetManager& presetManager) : manager (presetManager),
                                                          prevPresetButton ("", juce::DrawableButton::ImageOnButtonBackground),
                                                          nextPresetButton ("", juce::DrawableButton::ImageOnButtonBackground)
{
    manager.addListener (this);

    presetBox.setName ("Preset Manager");
    presetBox.setTooltip ("Use this menu to save and load plugin presets");

    setColour (backgroundColourID, juce::Colours::grey);
    setColour (textColourID, juce::Colours::white);
    setColour (textHighlightColourID, juce::Colours::cyan);

    addAndMakeVisible (presetBox);
    presetBox.setColour (juce::ComboBox::backgroundColourId, juce::Colours::transparentWhite);
    presetBox.setColour (juce::ComboBox::textColourId, juce::Colours::white);
    presetBox.setColour (juce::ComboBox::outlineColourId, juce::Colours::transparentWhite);
    presetBox.setJustificationType (juce::Justification::centred);
    presetBox.setTextWhenNothingSelected ("No Preset selected...");
    presetListUpdated();

    addChildComponent (presetNameEditor);
    presetNameEditor.setColour (juce::TextEditor::backgroundColourId, juce::Colours::transparentWhite);
    presetNameEditor.setColour (juce::TextEditor::outlineColourId, juce::Colours::transparentWhite);
    presetNameEditor.setColour (juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentWhite);
    presetNameEditor.setColour (juce::TextEditor::textColourId, juce::Colours::white);
    presetNameEditor.setColour (juce::TextEditor::highlightColourId, juce::Colours::cyan);
    presetNameEditor.setColour (juce::CaretComponent::caretColourId, juce::Colours::cyan);
    presetNameEditor.setFont (juce::Font (16.0f).boldened());
    presetNameEditor.setMultiLine (false, false);
    presetNameEditor.setJustification (juce::Justification::centred);

    auto setupNextPrevButton = [=] (juce::DrawableButton& button, bool forward) {
        addAndMakeVisible (button);
        button.setWantsKeyboardFocus (false);
        button.setColour (juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
        button.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        button.onClick = [=] { goToNextPreset (forward); };
    };

    setupNextPrevButton (prevPresetButton, false);
    setupNextPrevButton (nextPresetButton, true);

    updatePresetBoxText();
}

PresetsComp::~PresetsComp()
{
    manager.removeListener (this);
}

void PresetsComp::goToNextPreset (bool forward)
{
    const auto numPresets = manager.getNumPresets();
    auto idx = manager.getCurrentPresetIndex() + (forward ? 1 : -1);
    while (idx < 0)
        idx += numPresets;
    while (idx >= numPresets)
        idx -= numPresets;

    manager.loadPresetFromIndex (idx);
}

void PresetsComp::setNextPrevButton (const juce::Drawable* image, bool isNext)
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

void PresetsComp::paint (juce::Graphics& g)
{
    constexpr auto cornerSize = 5.0f;

    // update colours
    auto backgroundColour = findColour (backgroundColourID);
    presetBox.setColour (juce::PopupMenu::backgroundColourId, backgroundColour);
    presetNameEditor.setColour (juce::TextEditor::backgroundColourId, backgroundColour);

    auto textColour = findColour (textColourID);
    presetBox.setColour (juce::PopupMenu::textColourId, textColour);
    presetNameEditor.setColour (juce::TextEditor::textColourId, textColour);

    auto highlightColour = findColour (textHighlightColourID);
    presetNameEditor.setColour (juce::TextEditor::highlightColourId, highlightColour);
    presetNameEditor.setColour (juce::CaretComponent::caretColourId, highlightColour);

    g.setColour (findColour (backgroundColourID));
    g.fillRoundedRectangle (getLocalBounds().reduced (arrowWidth + arrowPad, 0).toFloat(), cornerSize);
}

void PresetsComp::resized()
{
    auto b = getLocalBounds();
    prevPresetButton.setBounds (b.removeFromLeft (arrowWidth));
    nextPresetButton.setBounds (b.removeFromRight (arrowWidth));

    juce::Rectangle<int> presetsBound (b.reduced (arrowPad, 0));
    presetBox.setBounds (presetsBound);
    presetNameEditor.setBounds (presetsBound);
    repaint();
}

void PresetsComp::presetListUpdated()
{
    presetBox.getRootMenu()->clear();

    std::map<juce::String, juce::PopupMenu> presetMapItems;
    int optionID = 0;
    for (const auto& presetIDPair : manager.getPresetMap())
    {
        const auto presetID = presetIDPair.first;
        const auto& preset = presetIDPair.second;

        juce::PopupMenu::Item presetItem { preset.getName() };
        presetItem.itemID = presetID + 1;
        presetItem.action = [=, &preset] {
            updatePresetBoxText();
            manager.loadPreset (preset);
        };

        presetMapItems[preset.getVendor()].addItem (presetItem);

        optionID = juce::jmax (optionID, presetItem.itemID);
    }

    for (auto& [vendorName, menu] : presetMapItems)
        presetBox.getRootMenu()->addSubMenu (vendorName, menu);

    addPresetOptions (optionID);
}

int PresetsComp::addPresetOptions (int optionID)
{
    auto menu = presetBox.getRootMenu();
    menu->addSeparator();

    juce::PopupMenu::Item saveItem { "Save Preset" };
    saveItem.itemID = ++optionID;
    saveItem.action = [=] {
        updatePresetBoxText();
        saveUserPreset();
    };
    menu->addItem (saveItem);

#if ! JUCE_IOS
    juce::PopupMenu::Item goToFolderItem { "Go to Preset folder..." };
    goToFolderItem.itemID = ++optionID;
    goToFolderItem.action = [=] {
        updatePresetBoxText();
        auto folder = manager.getUserPresetPath();
        if (folder.isDirectory())
            folder.startAsProcess();
        else
            chooseUserPresetFolder();
    };
    menu->addItem (goToFolderItem);

    juce::PopupMenu::Item chooseFolderItem { "Choose Preset folder..." };
    chooseFolderItem.itemID = ++optionID;
    chooseFolderItem.action = [=] {
        updatePresetBoxText();
        chooseUserPresetFolder();
    };
    menu->addItem (chooseFolderItem);
#endif

    return optionID;
}

void PresetsComp::chooseUserPresetFolder (std::function<void()> onFinish)
{
    constexpr auto folderChooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;
    fileChooser = std::make_shared<juce::FileChooser> ("Choose User Preset Folder");
    {
        fileChooser->launchAsync (folderChooserFlags, [=] (const juce::FileChooser& chooser) {
            manager.setUserPresetPath (chooser.getResult());

            if (onFinish != nullptr)
                onFinish();
        });
    }
}

void PresetsComp::saveUserPreset()
{
    presetNameEditor.setVisible (true);
    presetNameEditor.toFront (true);
    presetNameEditor.setText ("MyPreset");
    presetNameEditor.setHighlightedRegion ({ 0, 10 });

    presetNameEditor.onReturnKey = [=] {
        presetNameEditor.setVisible (false);

        auto presetName = presetNameEditor.getText();
        auto presetPath = manager.getUserPresetPath();
        if (presetPath == juce::File() || ! presetPath.isDirectory())
        {
            presetPath.deleteRecursively();
            chooseUserPresetFolder ([=] { savePresetFile (presetName + presetExt); });
        }
        else
        {
            savePresetFile (presetName + presetExt);
        }
    };

    presetNameEditor.onEscapeKey = [=] {
        presetNameEditor.setVisible (false);
        updatePresetBoxText();
    };
}

void PresetsComp::savePresetFile (const juce::String& fileName)
{
    auto presetPath = manager.getUserPresetPath();
    if (presetPath == juce::File() || ! presetPath.isDirectory())
        return;

    manager.saveUserPreset (presetPath.getChildFile (fileName));
}

void PresetsComp::updatePresetBoxText()
{
    auto* currentPreset = manager.getCurrentPreset();
    auto name = currentPreset == nullptr ? juce::String() : manager.getCurrentPreset()->getName();
    if (manager.getIsDirty())
        name += "*";

    juce::MessageManagerLock mml;
    presetBox.setText (name, juce::dontSendNotification);
}

} // namespace chowdsp
