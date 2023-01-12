#pragma once

#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_utils

#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>

namespace chowdsp::PresetsFrontend
{
class SettingsInterface
{
    using SettingID = GlobalPluginSettings::SettingID;
public:
    SettingsInterface (PresetManager& manager,
                       GlobalPluginSettings& settings,
                       const juce::File& userPresetsDir);

    void setUserPresetsPath (const juce::File& userPresetsPath);

private:
    void globalSettingChanged (SettingID);

    static constexpr SettingID userPresetsDirID = "chowdsp_presets_user_presets_dir";

    PresetManager& presetManager;
    GlobalPluginSettings& pluginSettings;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsInterface)
};
}

#endif
