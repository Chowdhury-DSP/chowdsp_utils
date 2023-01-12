#pragma once

namespace chowdsp
{
class PresetManager
{
public:
    template <typename PluginStateType>
    explicit PresetManager (PluginStateType& state,
                            juce::AudioProcessor* plugin = nullptr,
                            std::vector<juce::RangedAudioParameter*>&& presetAgnosticParams = {},
                            PresetTree::InsertionHelper&& insertionHelper = { nullptr, nullptr, nullptr })
        : processor (plugin),
          pluginState (state),
          presetAgnosticParameters (std::move (presetAgnosticParams)),
          presetTree (std::move (insertionHelper))
    {
        state.nonParams.addStateValues ({ &currentPreset, &isPresetDirty });
        initializeListeners (state.params, state.getParameterListeners());
    }

    virtual ~PresetManager() = default;

    /** Loads a preset by reference. */
    void loadPreset (const Preset& preset);

    /** Returns the currently loaded preset, or nullptr if no preset is loaded. */
    [[nodiscard]] const Preset* getCurrentPreset() const { return currentPreset.get(); }

    /** Returns true if the currently loaded preset is "dirty". */
    [[nodiscard]] bool getIsPresetDirty() const noexcept { return isPresetDirty.get(); }

    /** Adds a vector of presets. */
    void addPresets (std::vector<Preset>&& presets, bool areFactoryPresets = true);

    /**
     * Saves the plugin's current state to a preset file, and loads the preset.
     * The preset will have the same name as the file.
     */
    void saveUserPreset (const juce::File& file);

    /** Saves the given preset to a preset file, and loads the preset */
    void saveUserPreset (const juce::File& file, Preset&& preset);

    /**
     * Returns a json object containing the plugin's preset state.
     * Override this if your presets need custom state-saving behaviour
     */
    virtual nlohmann::json savePresetState();

    /**
     * Selects a preset to be the default preset.
     * If the preset is not already a factory preset, this function will add it.
     */
    void setDefaultPreset (Preset&& defaultPreset);

    /** Returns a pointer to the default preset. */
    const Preset* getDefaultPreset() const noexcept { return defaultPreset; }

    /** Loads the default preset. */
    void loadDefaultPreset();

    /** Returns true if the given preset is a factory preset. */
    bool isFactoryPreset (const Preset& preset) const;

    [[nodiscard]] auto& getPresetTree() { return presetTree; }
    [[nodiscard]] const auto& getPresetTree() const { return presetTree; }

    void setUserPresetPath (const juce::File& file);
    [[nodiscard]] juce::File getUserPresetPath() const;

    virtual void loadUserPresetsFromFolder (const juce::File& file);

    [[nodiscard]] juce::String getUserPresetVendorName() const noexcept { return userPresetsVendor; }

    Broadcaster<void()> presetListUpdatedBroadcaster;
    Broadcaster<void()> presetChangedBroadcaster;
    Broadcaster<void()> presetDirtyStatusBroadcaster;

protected:
    /** Override this if your presets need custom state-loading behaviour */
    virtual void loadPresetState (const nlohmann::json& state);

    /** Returns true if the given parameter is preset-agnostic */
    bool isPresetAgnosticParameter (const juce::RangedAudioParameter& param) const;

    /** Override this to support backwards compatibility for user presets */
    [[nodiscard]] virtual Preset loadUserPresetFromFile (const juce::File& file);

    PresetState currentPreset;
    StateValue<bool> isPresetDirty { "chowdsp_is_preset_dirty", false };

private:
    void initializeListeners (ParamHolder& params, ParameterListeners& listeners);

    juce::AudioProcessor* processor = nullptr;
    chowdsp::PluginState& pluginState;

    const std::vector<juce::RangedAudioParameter*> presetAgnosticParameters;
    chowdsp::ScopedCallbackList listeners;

    PresetTree presetTree;
    const Preset* defaultPreset = nullptr;
    std::vector<const Preset*> factoryPresets;

    juce::File userPresetPath {};
    juce::String userPresetsVendor { "User" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetManager)
};
}
