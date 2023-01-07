#pragma once

namespace chowdsp
{
class PresetManager
{
public:
    template <typename PluginStateType>
    explicit PresetManager (PluginStateType& state)
    {
        state.nonParams.addStateValues ({ &isDirty });
    }

    void setUserPresetConfigFile (const juce::String& presetConfigFilePath);
    juce::File getUserPresetConfigFile() const;

    void setUserPresetPath (const juce::File& file);
    juce::File getUserPresetPath() const;

    virtual void loadUserPresetsFromFolder (const juce::File& file);

private:
    StateValue<bool> isDirty { "is_preset_dirty", false };

    PresetTree presetTree;

    juce::String userPresetConfigPath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetManager)
};
}
