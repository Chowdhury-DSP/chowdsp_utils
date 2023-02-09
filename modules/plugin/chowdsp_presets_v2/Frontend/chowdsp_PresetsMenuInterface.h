#pragma once

namespace chowdsp::presets::frontend
{
/** Interface for interacting with a presets systems from a juce::PopupMenu */
class MenuInterface
{
public:
    explicit MenuInterface (PresetManager& manager, FileInterface* fileInterface = nullptr);

    /**
     * Adds all the available presets to the menu.
     *
     * Presets will be organized by whatever methods have been supplied to
     * the associated PresetTree.
     */
    void addPresetsToMenu (juce::PopupMenu& menu) const;

    /** Extra menu items that can be added to the menu. */
    enum ExtraMenuItems
    {
        Reset, /**< Reset the plugin to the current preset state */
        Save_Preset_As, /**< Save the current plugin state as a preset */
        Resave_Preset, /**< Resave the current preset */
        Delete_Preset, /**< Delete the current preset */
        Copy_Current_Preset, /**< Copies the current preset to the system clipboard */
        Paste_Preset, /**< Loads a preset from the system clipboard */
        Load_Preset_From_File, /**< Loads a preset from a file */
        Go_to_User_Presets_Folder, /**< Opens the user presets folder */
        Choose_User_Presets_Folder, /**< Opens a dialog for the user to choose the user presets directory */
        Separator, /**< A menu separator line */
    };

    /** Adds additional items to the menu. */
    void addExtraMenuItems (juce::PopupMenu& menu, std::initializer_list<ExtraMenuItems> extraMenuItems);

private:
    void refreshPresetsMenu();

    PresetManager& presetManager;
    FileInterface* fileInterface;
    ClipboardInterface clipboardInterface;

    juce::PopupMenu presetsMenu {};

    std::shared_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MenuInterface)
};
} // namespace chowdsp::presets::frontend
