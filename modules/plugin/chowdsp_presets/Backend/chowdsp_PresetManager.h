#pragma once

#include "chowdsp_Preset.h"

namespace chowdsp
{
/**
 * Class to manage presets for a plugin. This class can:
 *   - load and organize factory presets
 *   - save and load user presets
 *   - save and load presets as part of the plugin state
 *   - set and load a default preset
 *   - mark when a preset is "dirty"
 *   - interface with the AudioProcessor "program" API
 */
class PresetManager : private juce::AudioProcessorValueTreeState::Listener
{
public:
    /** Creates a new preset manager for an APVTS */
    explicit PresetManager (juce::AudioProcessorValueTreeState& vts);
    ~PresetManager() override;

    /** Loads a preset for a given index. */
    void loadPresetFromIndex (int index);

    /** Loads a preset by reference. */
    void loadPreset (const Preset& preset);

    /** Adds a vector of factory presets. Note that the vector will no longer be valid after being used in this function. */
    void addPresets (std::vector<Preset>& presets);

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

    /** Returns true if the plugin state has changed since loading the last preset */
    bool getIsDirty() const noexcept { return isDirty; }

    /** Call this function to tell the preset manager if the plugin state is "dirty" */
    void setIsDirty (bool shouldBeDirty);

    /** Returns the number of presets that are currently available */
    [[nodiscard]] int getNumPresets() const noexcept { return (int) presetMap.size(); }

    /** Returns the index of the currently selected preset */
    [[nodiscard]] int getCurrentPresetIndex() const noexcept { return getIndexForPreset (*currentPreset); }

    /** Returns the name of the preset at this index */
    [[nodiscard]] juce::String getPresetName (int index) const noexcept { return getPresetForIndex (index)->getName(); }

    /** Sets a file path to use to store user preset configurations */
    void setUserPresetConfigFile (const juce::String& presetConfigFilePath);

    /** Returns the user's preset configuration file */
    [[nodiscard]] juce::File getUserPresetConfigFile() const;

    /** Returns the user's preset configuration file */
    [[nodiscard]] static juce::File getUserPresetConfigFile (const juce::String& presetConfigFilePath);

    /** Returns the user's preset directory */
    [[nodiscard]] juce::File getUserPresetPath() const;

    /** Returns the user's preset directory */
    [[nodiscard]] static juce::File getUserPresetPath (const juce::String& presetConfigFilePath);

    /** Set's the user's preset directory */
    void setUserPresetPath (const juce::File& file);

    /** Load's user presets from the given directory */
    virtual void loadUserPresetsFromFolder (const juce::File& file);

    /** Saves the preset manager state to an XML element */
    [[nodiscard]] std::unique_ptr<juce::XmlElement> saveXmlState() const;

    /** Loads the preset manager state from an XML element */
    void loadXmlState (const juce::XmlElement* xml);

    static const juce::Identifier presetStateTag;

    /** Returns the map used to store the available presets */
    [[nodiscard]] const auto& getPresetMap() const { return presetMap; }

    /** Tell listeners that the preset list has been updated */
    void triggerPresetListUpdate() { listeners.call (&Listener::presetListUpdated); }

    /** Set the name to use for user presets */
    virtual void setUserPresetName (const juce::String& newName);

    /** Returns the name being used for user presets */
    [[nodiscard]] juce::String getUserPresetName() const noexcept { return userPresetsName; }

    /** Returns a vector of all the user-saved presets */
    [[nodiscard]] std::vector<const Preset*> getUserPresets() const;

    /** Attempts to load a preset with a custom failure callback, if the preset fails to load. */
    template <typename FailureCallback>
    void loadPresetSafe (std::unique_ptr<Preset> presetToLoad, FailureCallback&& failureCallback)
    {
        if (presetToLoad == nullptr || ! presetToLoad->isValid())
        {
            failureCallback();
            return;
        }

        keepAlivePreset = std::move (presetToLoad);
        loadPreset (*keepAlivePreset);
    }

    /** Listener class to hear alerts about preset manager changes */
    struct Listener
    {
        virtual ~Listener() = default;

        /** Called whenever the list of available presets is changed */
        virtual void presetListUpdated() {}

        /** Called when the preset "dirty" status is changed */
        virtual void presetDirtyStatusChanged() {}

        /** Called when the selected preset is changed */
        virtual void selectedPresetChanged() {}
    };

    void addListener (Listener* l) { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

protected:
    /** Override this if your presets need custom state-saving behaviour */
    virtual std::unique_ptr<juce::XmlElement> savePresetState();

    /** Override this if your presets need custom state-loading behaviour */
    virtual void loadPresetState (const juce::XmlElement* xml);

    /** Override this to suppor backwards compatibility for user presets */
    virtual Preset loadUserPresetFromFile (const juce::File& file);

    juce::AudioProcessorValueTreeState& vts;
    juce::AudioProcessor& processor;

    enum
    {
        factoryUserNumIDs = 1000,
        userUserIDStart = 1000000,
    };

    std::map<int, Preset> presetMap;
    std::unordered_map<juce::String, int> userIDMap;

    juce::String userPresetsName;
    std::unique_ptr<Preset> keepAlivePreset;
    const Preset* currentPreset = nullptr;

    juce::ListenerList<Listener> listeners;

private:
    void parameterChanged (const juce::String& parameterID, float newValue) override;
    std::pair<const int, Preset>& addFactoryPreset (Preset&& preset);
    [[nodiscard]] const Preset* findPreset (const Preset& preset) const;

    [[nodiscard]] int getIndexForPreset (const Preset& preset) const;
    [[nodiscard]] const Preset* getPresetForIndex (int index) const;

    bool isDirty = false;

    const Preset* defaultPreset = nullptr;

    juce::String userPresetConfigPath;

    static const juce::Identifier presetDirtyTag;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetManager)
};

} // namespace chowdsp
