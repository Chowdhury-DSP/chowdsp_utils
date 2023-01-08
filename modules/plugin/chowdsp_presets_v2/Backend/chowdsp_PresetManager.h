#pragma once

namespace chowdsp
{
class PresetManager
{
public:
    template <typename PluginStateType>
    explicit PresetManager (PluginStateType& state,
                            juce::AudioProcessor* plugin = nullptr,
                            std::vector<juce::RangedAudioParameter*>&& presetAgnosticParams = {})
        : processor (plugin),
          pluginState (state),
          presetAgnosticParameters (std::move (presetAgnosticParams))
    {
        state.nonParams.addStateValues ({ &isPresetDirty });
        initializeParameterListeners (state.params, state.getParameterListeners());
    }

    virtual ~PresetManager() = default;

    /** Loads a preset by reference. */
    void loadPreset (const Preset& preset);

    /** Adds a vector of factory presets. */
    void addPresets (std::vector<Preset>&& presets);

    /** Saves the plugin's current state to a preset file, and loads the preset */
    void saveUserPreset (const juce::File& file);

    /** Returns a pointer to the currently selected preset */
    const Preset* getCurrentPreset() const noexcept { return currentPreset; }

    /**
     * Selects a preset to be the default preset.
     * If the preset is not already a factory preset, this function will add it.
     */
    void setDefaultPreset (Preset&& defaultPreset);

    /** Returns a pointer to the default preset. */
    const Preset* getDefaultPreset() const noexcept { return defaultPreset; }

    /** Loads the default preset. */
    void loadDefaultPreset();

    [[nodiscard]] auto& getPresetTree() { return presetTree; }
    [[nodiscard]] const auto& getPresetTree() const { return presetTree; }

    void setUserPresetConfigFile (const juce::String& presetConfigFilePath);
    [[nodiscard]] juce::File getUserPresetConfigFile() const;

    void setUserPresetPath (const juce::File& file);
    [[nodiscard]] juce::File getUserPresetPath() const;

    virtual void loadUserPresetsFromFolder (const juce::File& file);

    chowdsp::Broadcaster<void()> presetListUpdatedBroadcaster;
    chowdsp::Broadcaster<void()> currentPresetChangedBroadcaster;
    StateValue<bool> isPresetDirty { "is_preset_dirty", false };

protected:
    /** Override this if your presets need custom state-saving behaviour */
    virtual nlohmann::json savePresetState();

    /** Override this if your presets need custom state-loading behaviour */
    virtual void loadPresetState (const nlohmann::json& state);

    /** Override this to support backwards compatibility for user presets */
    [[nodiscard]] virtual Preset loadUserPresetFromFile (const juce::File& file);

    const Preset* currentPreset = nullptr;

private:
    void initializeParameterListeners (ParamHolder& params, ParameterListeners& listeners);

    juce::AudioProcessor* processor = nullptr;
    chowdsp::PluginState& pluginState;

    const std::vector<juce::RangedAudioParameter*> presetAgnosticParameters;
    chowdsp::ScopedCallbackList parameterListeners;

    PresetTree presetTree;
    const Preset* defaultPreset = nullptr;

    juce::String userPresetConfigPath;
    juce::String userPresetsVendor { "User" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetManager)
};
}
