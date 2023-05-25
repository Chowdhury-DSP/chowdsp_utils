#pragma once

namespace chowdsp::presets::frontend
{
class SettingsInterface;

/** Interface for working with preset files. */
class FileInterface
{
public:
    explicit FileInterface (PresetManager& manager,
                            SettingsInterface* settingsInterface = nullptr);
    virtual ~FileInterface() = default;

    /** Saves a user preset using `savePresetCallback()` */
    virtual void savePreset();

    /**
     * Resaves the current preset.
     *
     * The caller is responsible for making sure that the preset
     * being resaved is not a factory preset, and that it actually
     * has a file associated with it.
     */
    virtual void resaveCurrentPreset();

    /** Deletes the currently loaded preset. */
    virtual void deleteCurrentPreset();

    /** Launches a file browser so that the user can load a preset from a file. */
    virtual void loadPresetFromFile();

    /** Launches the user presets folder in a file browser. */
    void goToUserPresetsFolder() const;

    /** Launches a file browser so that the user can choose their user presets folder. */
    virtual void chooseUserPresetsFolder();

    /** Called when a preset needs to be saved. */
    std::function<void (nlohmann::json&&)> savePresetCallback;

    /** Called to check if the user _actually_ wants to delete a preset. */
    std::function<void (const Preset&, std::function<void (const Preset&)>&&)> confirmAndDeletePresetCallback;

    /** Called when a preset fails to load. Note that the supplied preset may not be valid. */
    std::function<void (const Preset&)> failedToLoadPresetCallback;

    /** Returns a file which can be used to save a preset. */
    [[nodiscard]] juce::File getFileForPreset (const Preset& preset) const;

    /** Returns true if a SettingsInterface has been supplied. */
    [[nodiscard]] bool hasSettingsInterface() const noexcept { return settingsInterface != nullptr; }

protected:
    PresetManager& presetManager;

    std::shared_ptr<juce::FileChooser> fileChooser;

    SettingsInterface* settingsInterface = nullptr;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileInterface)
};
} // namespace chowdsp::presets::frontend
