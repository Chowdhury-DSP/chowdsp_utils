#pragma once

namespace chowdsp::presets
{
/** A class for managing a plugin's presets system. */
class PresetManager
{
public:
    /** Creates a PresetManager for a given plugin state. */
    explicit PresetManager (PluginState& state,
                            juce::AudioProcessor* plugin = nullptr,
                            const juce::String& presetFileExtension = ".preset",
                            std::vector<juce::RangedAudioParameter*>&& presetAgnosticParams = {},
                            PresetTree::InsertionHelper&& insertionHelper = { nullptr, nullptr, nullptr });
    virtual ~PresetManager() = default;

    /** Loads a preset by reference. */
    void loadPreset (const Preset& preset);

#if HAS_CLAP_JUCE_EXTENSIONS
    /** Loads a preset based on information provided by the CLAP preset-load extension. */
    virtual bool loadCLAPPreset (uint32_t location_kind, const char* location, const char* load_key) noexcept;

    Broadcaster<void (uint32_t location_kind, const char* location, const char* load_key)> clapPresetLoadedBroadcaster {};
#endif

    /** Returns the currently loaded preset, or nullptr if no preset is loaded. */
    [[nodiscard]] const Preset* getCurrentPreset() const { return saverLoader.getCurrentPreset(); }

    /** Returns true if the currently loaded preset is "dirty". */
    [[nodiscard]] bool getIsPresetDirty() const noexcept { return saverLoader.getIsPresetDirty(); }

    /** Adds a vector of presets. Note that all factory presets should be added at once (and only once). */
    void addPresets (std::vector<Preset>&& presets, bool areFactoryPresets = true);

    /** Returns a user preset with the given name and state. */
    [[nodiscard]] virtual Preset getUserPresetForState (const juce::String& presetName, nlohmann::json&& presetState) const;

    /**
     * Saves the plugin's current state to a preset file, and loads the preset.
     * The preset will have the same name as the file.
     */
    void saveUserPreset (const juce::File& file);

    /** Saves the given preset to a preset file, and loads the preset */
    void saveUserPreset (const juce::File& file, Preset&& preset);

    /**
     * Selects a preset to be the default preset.
     * If the preset is not already a factory preset, this function will add it.
     */
    void setDefaultPreset (Preset&& defaultPreset);

    /** Returns a pointer to the default preset. */
    [[nodiscard]] const Preset* getDefaultPreset() const noexcept { return defaultPreset; }

    /** Loads the default preset. */
    void loadDefaultPreset();

    /** Returns the internal preset tree. */
    [[nodiscard]] auto& getPresetTree() { return presetTree; }

    /** Returns the internal preset tree. */
    [[nodiscard]] const auto& getPresetTree() const { return presetTree; }

    /** Returns a vector of factory presets. */
    [[nodiscard]] const auto& getFactoryPresets() const { return factoryPresets; }

    /** Set's the user preset path. This will force any user presets to be re-scanned. */
    void setUserPresetPath (const juce::File& file);

    /** Returns the user preset path. */
    [[nodiscard]] juce::File getUserPresetPath() const;

    /** Returns the file extension used for saving preset files. */
    [[nodiscard]] juce::String getPresetFileExtension() const noexcept { return presetFileExt; }

    /** Returns the "Vendor" name used for user presets. */
    [[nodiscard]] juce::String getUserPresetVendorName() const noexcept { return userPresetsVendor; }

    /** Loads a set of user presets from the given folder path. */
    virtual void loadUserPresetsFromFolder (const juce::File& file);

    /** Returns the PresetSaverLoader  */
    auto& getSaveLoadHelper() { return saverLoader; }

    /** Called whenever the preset list has changed. */
    Broadcaster<void()> presetListUpdatedBroadcaster {};

protected:
    /** Override this to support backwards compatibility for user presets */
    [[nodiscard]] virtual Preset loadUserPresetFromFile (const juce::File& file);

    juce::String userPresetsVendor { "User" };

    PresetSaverLoader saverLoader;

private:
    PresetTree presetTree;
    std::vector<Preset> factoryPresets;
    const Preset* defaultPreset = nullptr;

    juce::File userPresetPath {};
    const juce::String presetFileExt {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetManager)
};
} // namespace chowdsp::presets
