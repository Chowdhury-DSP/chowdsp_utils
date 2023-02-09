#include "chowdsp_PresetsSettingsInterface.h"

#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_utils
namespace chowdsp::presets::frontend
{
SettingsInterface::SettingsInterface (PresetManager& manager,
                                      GlobalPluginSettings& settings,
                                      const juce::File& userPresetsDir)
    : presetManager (manager),
      pluginSettings (settings)
{
    jassert (userPresetsDir != juce::File {});
    settings.addProperties<&SettingsInterface::globalSettingChanged> (
        { { userPresetsDirID, userPresetsDir.getFullPathName() } },
        *this);
    globalSettingChanged (userPresetsDirID);
}

void SettingsInterface::setUserPresetsPath (const juce::File& userPresetsPath)
{
    pluginSettings.setProperty<juce::String> (userPresetsDirID, userPresetsPath.getFullPathName());
}

void SettingsInterface::globalSettingChanged (SettingID settingID)
{
    if (settingID != userPresetsDirID)
        return;

    const auto newUserPresetsPath = juce::File { pluginSettings.getProperty<juce::String> (userPresetsDirID) };
    if (presetManager.getUserPresetPath() == newUserPresetsPath)
        return;

    if (newUserPresetsPath.existsAsFile())
        newUserPresetsPath.deleteFile();

    if (! newUserPresetsPath.isDirectory())
        newUserPresetsPath.createDirectory();

    presetManager.setUserPresetPath (newUserPresetsPath);
}
} // namespace chowdsp::presets::frontend
#endif
