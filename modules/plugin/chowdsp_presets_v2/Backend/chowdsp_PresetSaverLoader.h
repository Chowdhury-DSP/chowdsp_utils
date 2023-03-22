#pragma once

namespace chowdsp::presets
{
/** Helper class for saving and loading plugin presets. */
class PresetSaverLoader
{
public:
    explicit PresetSaverLoader (PluginState& state,
                                juce::AudioProcessor* plugin = nullptr,
                                std::vector<juce::RangedAudioParameter*>&& presetAgnosticParams = {});

    /** Returns the currently loaded preset, or nullptr if no preset is loaded. */
    [[nodiscard]] const Preset* getCurrentPreset() const { return currentPreset.get(); }

    /** Returns the preset state object used by this class. */
    [[nodiscard]] PresetState& getPresetState() { return currentPreset; }

    /** Returns true if the currently loaded preset is "dirty". */
    [[nodiscard]] bool getIsPresetDirty() const noexcept { return isPresetDirty.get(); }

    /** Loads a preset by reference. */
    void loadPreset (const Preset& preset);

    /** Returns true if the given parameter is preset-agnostic */
    bool isPresetAgnosticParameter (const juce::RangedAudioParameter& param) const;

    /** Returns a json object containing the plugin's preset state. */
    std::function<nlohmann::json()> savePresetState = [this]
    { return savePresetParameters(); };

    /** Sets the plugin state from a given preset state. */
    std::function<void (const nlohmann::json& state)> loadPresetState = [this] (const nlohmann::json& state)
    { loadPresetParameters (state); };

    /** Saves the parameters from a preset state. */
    nlohmann::json savePresetParameters();

    /** Loads the parameters from a preset state. */
    void loadPresetParameters (const nlohmann::json& state);

    /** Called when the current preset has changed. */
    Broadcaster<void()> presetChangedBroadcaster {};

    /** Called whenever the current preset's "dirty" status has changed. */
    Broadcaster<void()> presetDirtyStatusBroadcaster {};

private:
    void initializeListeners (ParamHolder& params, ParameterListeners& listeners);

    PresetState currentPreset;
    StateValue<bool> isPresetDirty { "chowdsp_is_preset_dirty", false };

    juce::AudioProcessor* processor = nullptr;
    chowdsp::PluginState& pluginState;

    const std::vector<juce::RangedAudioParameter*> presetAgnosticParameters {};
    chowdsp::ScopedCallbackList listeners;

    bool areWeInTheMidstOfAPresetChange = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetSaverLoader)
};
} // namespace chowdsp::presets
