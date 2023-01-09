#pragma once

namespace chowdsp::PresetsFrontend
{
class FileInterface
{
public:
    FileInterface (PresetManager& manager, const juce::String& presetFileExtension);

    void savePreset();
    void resaveCurrentPreset();
    void deleteCurrentPreset();
    void loadPresetFromFile();

    void goToUserPresetsFolder();
    void chooseUserPresetsFolder();

    std::function<std::pair<Preset, juce::File> (nlohmann::json&&)> savePresetCallback;
    std::function<bool (const Preset&)> checkDeletePresetCallback;
    std::function<void (const Preset&)> failedToLoadPresetCallback;

    [[nodiscard]] juce::File getFileForPreset (const Preset& preset) const;

private:
    PresetManager& presetManager;

    std::shared_ptr<juce::FileChooser> fileChooser;

    const juce::String presetFileExtension;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileInterface)
};
} // namespace chowdsp::PresetsFrontend
