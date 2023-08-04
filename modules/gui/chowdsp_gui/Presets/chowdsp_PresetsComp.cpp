#include "chowdsp_PresetsComp.h"

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
    presetBox.setDescription ("Preset Manager");
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

    auto setupNextPrevButton = [this] (juce::DrawableButton& button, bool forward)
    {
        addAndMakeVisible (button);
        button.setWantsKeyboardFocus (false);
        button.setTitle ("Go to " + juce::String (forward ? "next" : "previous") + " preset");
        button.setColour (juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
        button.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        button.onClick = [this, forward]
        { goToNextPreset (forward); };
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

void PresetsComp::selectedPresetChanged()
{
    updatePresetBoxText();
    presetListUpdated();
}

void PresetsComp::presetListUpdated()
{
    presetBox.getRootMenu()->clear();

    int optionID = 0;
    optionID = createPresetsMenu (optionID);
    optionID = addSavePresetOptions (optionID);
    optionID = addSharePresetOptions (optionID);

#if ! JUCE_IOS
    optionID = addPresetFolderOptions (optionID);
#endif

    juce::ignoreUnused (optionID);
}

int PresetsComp::createPresetsMenu (int optionID)
{
    struct VendorPresetCollection
    {
        std::map<juce::String, juce::PopupMenu> categoryPresetMenus;
        std::vector<juce::PopupMenu::Item> nonCategoryItems;
    };

    std::map<juce::String, VendorPresetCollection> presetMapItems;
    for (const auto& presetIDPair : manager.getPresetMap())
    {
        const auto presetID = presetIDPair.first;
        const auto& preset = presetIDPair.second;

        juce::PopupMenu::Item presetItem { preset.getName() };
        presetItem.itemID = presetID + 1;
        presetItem.action = [this, &preset]
        {
            updatePresetBoxText();
            manager.loadPreset (preset);
        };

        const auto& presetCategory = preset.getCategory();
        if (presetCategory.isEmpty())
            presetMapItems[preset.getVendor()].nonCategoryItems.push_back (presetItem);
        else
            presetMapItems[preset.getVendor()].categoryPresetMenus[presetCategory].addItem (presetItem);

        optionID = juce::jmax (optionID, presetItem.itemID);
    }

    for (auto& [vendorName, vendorCollection] : presetMapItems)
    {
        juce::PopupMenu vendorMenu;
        for (const auto& [category, categoryMenu] : vendorCollection.categoryPresetMenus)
            vendorMenu.addSubMenu (category, categoryMenu);

        std::sort (vendorCollection.nonCategoryItems.begin(), vendorCollection.nonCategoryItems.end(), [] (auto& item1, auto& item2)
                   { return item1.text < item2.text; });
        for (const auto& extraItem : vendorCollection.nonCategoryItems)
            vendorMenu.addItem (extraItem);

        presetBox.getRootMenu()->addSubMenu (vendorName, vendorMenu);
    }

    return optionID;
}

int PresetsComp::addSavePresetOptions (int optionID)
{
    auto menu = presetBox.getRootMenu();
    menu->addSeparator();

    optionID = addPresetMenuItem (menu,
                                  optionID,
                                  "Reset",
                                  [&]
                                  {
                                      if (auto* currentPreset = manager.getCurrentPreset())
                                          manager.loadPreset (*currentPreset);
                                  });

    optionID = addPresetMenuItem (menu,
                                  optionID,
                                  "Save Preset As",
                                  [&]
                                  { saveUserPreset(); });

    if (auto* currentPreset = manager.getCurrentPreset())
    {
        // editing should only be allowed for user presets!
        if (currentPreset->getVendor() == manager.getUserPresetName() && currentPreset->getPresetFile().existsAsFile())
        {
            optionID = addPresetMenuItem (menu,
                                          optionID,
                                          "Resave Preset",
                                          [this]
                                          {
                                              if (auto* curPreset = manager.getCurrentPreset())
                                                  savePresetFile (curPreset->getPresetFile().getRelativePathFrom (manager.getUserPresetPath()));
                                          });
        }

        if (manager.getCurrentPreset()->getPresetFile() != juce::File())
        {
            optionID = addPresetMenuItem (menu,
                                          optionID,
                                          "Delete Preset",
                                          [this]
                                          {
                                              if (auto* curPreset = manager.getCurrentPreset())
                                              {
                                                  auto presetFile = curPreset->getPresetFile();
                                                  if (! (presetFile.existsAsFile() && presetFile.hasFileExtension (presetExt)))
                                                  {
                                                      juce::NativeMessageBox::showMessageBox (juce::MessageBoxIconType::WarningIcon, "Preset Deletion", "Unable to find preset file!");
                                                      return;
                                                  }

                                                  if (juce::NativeMessageBox::showOkCancelBox (juce::MessageBoxIconType::QuestionIcon, "Preset Deletion", "Are you sure you want to delete this preset? This operation cannot be undone."))
                                                  {
                                                      presetFile.deleteFile();
                                                      manager.loadDefaultPreset();
                                                      manager.loadUserPresetsFromFolder (manager.getUserPresetPath());
                                                  }
                                              }
                                          });
        }
    }

    return optionID;
}

int PresetsComp::addSharePresetOptions (int optionID)
{
    auto menu = presetBox.getRootMenu();
    menu->addSeparator();

    optionID = addPresetMenuItem (menu,
                                  optionID,
                                  "Copy Current Preset",
                                  [&]
                                  {
                                      if (auto* currentPreset = manager.getCurrentPreset())
                                          juce::SystemClipboard::copyTextToClipboard (currentPreset->toXml()->toString());
                                  });

    optionID = addPresetMenuItem (menu,
                                  optionID,
                                  "Paste Preset",
                                  [&]
                                  {
                                      const auto presetText = juce::SystemClipboard::getTextFromClipboard();
                                      if (presetText.isEmpty())
                                          return;

                                      if (auto presetXml = juce::XmlDocument::parse (presetText))
                                          loadPresetSafe (std::make_unique<Preset> (presetXml.get()));
                                  });

#if ! JUCE_IOS
    return addPresetMenuItem (menu, optionID, "Load Preset From File", [&]
                              {
        constexpr auto fileBrowserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
        fileChooser = std::make_shared<juce::FileChooser> ("Load Preset", manager.getUserPresetPath(), "*" + presetExt, true, false, getTopLevelComponent());
        fileChooser->launchAsync (fileBrowserFlags,
                                  [&] (const juce::FileChooser& fc) {
                                      if (fc.getResults().isEmpty())
                                          return;

                                      loadPresetSafe (std::make_unique<Preset> (fc.getResult()));
                                  }); });
#else
    return optionID;
#endif
}

int PresetsComp::addPresetFolderOptions (int optionID)
{
    auto menu = presetBox.getRootMenu();
    menu->addSeparator();

    if (manager.getUserPresetPath().isDirectory())
    {
        optionID = addPresetMenuItem (menu, optionID, "Go to Preset Folder...", [&]
                                      { manager.getUserPresetPath().startAsProcess(); });
    }

    return addPresetMenuItem (menu, optionID, "Choose Preset Folder...", [&]
                              { chooseUserPresetFolder ({}); });
}

void PresetsComp::loadPresetSafe (std::unique_ptr<Preset> preset)
{
    manager.loadPresetSafe (std::move (preset), []
                            { juce::MessageManager::callAsync (
                                  []
                                  {
                                      juce::NativeMessageBox::showAsync (juce::MessageBoxOptions()
                                                                             .withIconType (juce::MessageBoxIconType::WarningIcon)
                                                                             .withTitle ("Preset Load Failure")
                                                                             .withMessage ("Unable to load preset!")
                                                                             .withButton ("OK"),
                                                                         [] (int) {});
                                  }); });
}

void PresetsComp::chooseUserPresetFolder (const std::function<void()>& onFinish)
{
    constexpr auto folderChooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;
    fileChooser = std::make_shared<juce::FileChooser> ("Choose User Preset Folder");
    {
        fileChooser->launchAsync (folderChooserFlags, [this, onFinish] (const juce::FileChooser& chooser)
                                  {
            manager.setUserPresetPath (chooser.getResult());

            if (onFinish != nullptr)
                onFinish(); });
    }
}

void PresetsComp::saveUserPreset()
{
    presetNameEditor.setVisible (true);
    presetNameEditor.toFront (true);
    presetNameEditor.setText ("MyPreset");
    presetNameEditor.grabKeyboardFocus();
    presetNameEditor.setHighlightedRegion ({ 0, 100 });

    presetNameEditor.onReturnKey = [this]
    {
        presetNameEditor.setVisible (false);

        auto presetName = presetNameEditor.getText();
        auto presetPath = manager.getUserPresetPath();
        if (presetPath == juce::File() || ! presetPath.isDirectory())
        {
            presetPath.deleteRecursively();
            chooseUserPresetFolder ([this, presetName]
                                    { savePresetFile (presetName + presetExt); });
        }
        else
        {
            savePresetFile (presetName + presetExt);
        }
    };

    presetNameEditor.onEscapeKey = [this]
    {
        presetNameEditor.setVisible (false);
        updatePresetBoxText();
    };

    presetNameEditor.onFocusLost = [this]
    {
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

void PresetsComp::handleAsyncUpdate()
{
    if (presetBox.getText() != presetBoxText)
        presetBox.setText (presetBoxText, juce::dontSendNotification);
}

void PresetsComp::updatePresetBoxText()
{
    presetBoxText = {};
    if (auto* currentPreset = manager.getCurrentPreset())
    {
        if (currentPreset->isValid())
            presetBoxText = currentPreset->getName();
    }
    if (manager.getIsDirty())
        presetBoxText += "*";

    triggerAsyncUpdate();
}

} // namespace chowdsp
