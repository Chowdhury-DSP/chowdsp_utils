#pragma once

namespace chowdsp
{
class PresetManager : private juce::AudioProcessorValueTreeState::Listener
{
public:
    PresetManager (juce::AudioProcessorValueTreeState& vts);
    ~PresetManager() override;

    void loadPresetFromIndex (int index);
    void loadPreset (const Preset& preset);

    void addPresets (std::vector<Preset>& presets);
    void saveUserPreset (const juce::File& file);
    const Preset* getCurrentPreset() const noexcept { return currentPreset; }

    void setDefaultPreset (Preset&& defaultPreset);
    void loadDefaultPreset();

    void parameterChanged (const juce::String& parameterID, float newValue) override;
    bool getIsDirty() const noexcept { return isDirty; }

    int getNumPresets() const noexcept { return (int) presetMap.size(); }
    int getCurrentPresetIndex() const noexcept { return getIndexForPreset (*currentPreset); }
    juce::String getPresetName (int index) const noexcept { return getPresetForIndex (index)->getName(); }

    void setUserPresetConfigFile (const juce::String& presetConfigFilePath);
    juce::File getUserPresetConfigFile() const;
    juce::File getUserPresetPath() const;
    void setUserPresetPath (const juce::File& file);
    void loadUserPresetsFromFolder (const juce::File& file);

    std::unique_ptr<juce::XmlElement> saveXmlState() const;
    void loadXmlState (juce::XmlElement* xml);
    static const juce::Identifier presetStateTag;

    const std::unordered_map<int, Preset>& getPresetMap() const { return presetMap; }

    struct Listener
    {
        virtual ~Listener() = default;
        virtual void presetListUpdated() {}
        virtual void presetDirtyStatusChanged() {}
        virtual void selectedPresetChanged() {}
    };

    void addListener (Listener* l) { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

protected:
    virtual std::unique_ptr<juce::XmlElement> savePresetState();
    virtual void loadPresetState (const juce::XmlElement* xml);

    juce::AudioProcessorValueTreeState& vts;
    juce::AudioProcessor& processor;

private:
    std::pair<const int, Preset>& addFactoryPreset (Preset&& preset);
    void setIsDirty (bool shouldBeDirty);
    const Preset* findPreset (const Preset& preset) const;

    int getIndexForPreset (const Preset& preset) const;
    const Preset* getPresetForIndex (int index) const;

    enum
    {
        factoryUserNumIDs = 1000,
        userUserIDStart = 1000000,
    };

    std::unordered_map<int, Preset> presetMap;
    std::unordered_map<juce::String, int> userIDMap;

    juce::ListenerList<Listener> listeners;
    bool isDirty = false;

    const Preset* currentPreset = nullptr;
    const Preset* defaultPreset = nullptr;

    juce::String userPresetConfigPath;

    std::unique_ptr<Preset> statePreset;
    static const juce::Identifier presetDirtyTag;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetManager)
};

} // namespace chowdsp
