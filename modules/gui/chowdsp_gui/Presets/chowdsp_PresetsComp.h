#pragma once

#include <chowdsp_presets/chowdsp_presets.h>

namespace chowdsp
{
/** A component to display a presets menu */
class PresetsComp : public juce::Component,
                    private PresetManager::Listener,
                    private juce::AsyncUpdater
{
public:
    enum ColourIDs
    {
        backgroundColourID, /**< Colour used for the background of the preset menu */
        textColourID, /**< Colour used for the preset menu text */
        textHighlightColourID, /**< Colour used for the preset menu text highlighting */
    };

    /** Creates an Presets component for the given preset manager */
    explicit PresetsComp (PresetManager& presetManager);
    ~PresetsComp() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void handleAsyncUpdate() override;

    juce::String getPresetMenuText() const noexcept { return presetBox.getText(); }
    juce::ComboBox& getPresetMenuBox() { return presetBox; }
    juce::TextEditor& getPresetNameEditor() { return presetNameEditor; }

    void presetListUpdated() override;
    void presetDirtyStatusChanged() override { updatePresetBoxText(); }
    void selectedPresetChanged() override;

    void goToNextPreset (bool forward);
    void setNextPrevButton (const juce::Drawable* image, bool isNext);

protected:
    virtual void chooseUserPresetFolder (const std::function<void()>& onFinish);
    virtual int createPresetsMenu (int optionID);
    virtual int addSavePresetOptions (int optionID);
    virtual int addSharePresetOptions (int optionID);
    virtual int addPresetFolderOptions (int optionID);
    virtual void saveUserPreset();
    virtual void updatePresetBoxText();

    virtual void loadPresetSafe (std::unique_ptr<Preset> preset);

    template <typename ActionType>
    int addPresetMenuItem (juce::PopupMenu* menu, int optionID, const juce::String& itemText, ActionType&& action)
    {
        juce::PopupMenu::Item item { itemText };
        item.itemID = ++optionID;
        item.action = [&, forwardedAction = std::forward<ActionType> (action)]
        {
            updatePresetBoxText();
            forwardedAction();
        };
        menu->addItem (item);

        return optionID;
    }

    PresetManager& manager;

    juce::ComboBox presetBox;
    juce::TextEditor presetNameEditor;
    juce::DrawableButton prevPresetButton, nextPresetButton;

private:
    void savePresetFile (const juce::String& fileName);

    std::shared_ptr<juce::FileChooser> fileChooser;

    juce::String presetBoxText;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetsComp)
};

} // namespace chowdsp
