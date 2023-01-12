#pragma once

namespace chowdsp::PresetsFrontend
{
class SettingsInterface;
class FileInterface
{
public:
    FileInterface (PresetManager& manager,
                   const juce::String& presetFileExtension,
                   SettingsInterface* settingsInterface = nullptr);

    void savePreset();
    void resaveCurrentPreset();
    void deleteCurrentPreset();
    void loadPresetFromFile();

    void goToUserPresetsFolder();
    void chooseUserPresetsFolder();

    std::function<void (nlohmann::json&&)> savePresetCallback;
    std::function<bool (const Preset&)> checkDeletePresetCallback;
    std::function<void (const Preset&)> failedToLoadPresetCallback;

    [[nodiscard]] juce::File getFileForPreset (const Preset& preset) const;

    [[nodiscard]] bool hasSettingsInterface() const noexcept { return settingsInterface != nullptr; }

private:
    PresetManager& presetManager;

    std::shared_ptr<juce::FileChooser> fileChooser;

    const juce::String presetFileExtension;

    SettingsInterface* settingsInterface = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileInterface)
};
} // namespace chowdsp::PresetsFrontend
