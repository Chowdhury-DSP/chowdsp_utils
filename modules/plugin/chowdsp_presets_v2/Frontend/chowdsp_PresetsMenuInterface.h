#pragma once

namespace chowdsp::PresetsFrontend
{
class MenuInterface
{
public:
    explicit MenuInterface (PresetManager& manager);

    void addPresetsToMenu (juce::PopupMenu& menu);

    enum ExtraMenuItems
    {
        Reset, /**< Reset the plugin to the current preset state */
        Save_Preset_As, /**< Save the current plugin state as a preset */
        Resave_Preset, /**< Resave the current preset */
        Delete_Preset, /**< Delete the current preset */
        Copy_Current_Preset, /**< Copies the current preset to the system clipboard */
        Paste_Preset, /**< Loads a preset from the system clipboard */
        //        Load_Preset_From_File, /**< Loads a preset from a file */
        //        Go_to_User_Presets_Folder, /**< Opens the user presets folder */
        //        Choose_User_Presets_Folder, /**< Opens a dialog for the user to choose the user presets directory */
        Separator, /**< A menu separator line */
    };

    void addExtraMenuItems (juce::PopupMenu& menu, std::initializer_list<ExtraMenuItems> extraMenuItems);

    std::function<std::pair<Preset, juce::File> (nlohmann::json&&)> savePresetCallback;

private:
    void refreshPresetsMenu();

    PresetManager presetManager;

    juce::PopupMenu presetsMenu {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MenuInterface)
};
} // namespace chowdsp::PresetsFrontend
