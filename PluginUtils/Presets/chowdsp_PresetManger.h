#pragma once

namespace chowdsp
{
class PresetManager : private juce::AudioProcessorValueTreeState::Listener
{
public:
    PresetManager (juce::AudioProcessorValueTreeState& vts);
    ~PresetManager();

    void loadPresetFromIdx (int index);
    void loadPreset (const Preset& preset);

    void addPresets (std::vector<Preset>& presets);
    void saveUserPreset (const juce::File& file);
    const Preset* getCurrentPreset() const noexcept { return currentPreset; }

    void parameterChanged (const juce::String& parameterID, float newValue) override;
    bool getIsDirty() const noexcept { return isDirty; }

    int getNumPresets() const noexcept { return (int) presetMap.size(); }
    int getCurrentPresetIndex() const noexcept { return getIndexForPreset (*currentPreset); }
    juce::String getPresetName (int index) const noexcept { return getPresetForIndex (index)->name; }

    void setUserPresetConfigFile (const juce::String& presetConfigFilePath);
    juce::File getUserPresetConfigFile() const;
    juce::File getUserPresetPath() const;
    void setUserPresetPath (const juce::File& file);
    void loadUserPresetsFromFolder (const juce::File& file);

    std::unique_ptr<juce::XmlElement> saveXmlState() const;
    void loadXmlState (juce::XmlElement* xml);
    static const juce::Identifier presetStateTag;

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
    virtual void loadPresetState (juce::XmlElement* xml);

    juce::AudioProcessorValueTreeState& vts;
    juce::AudioProcessor& processor;

private:
    void addFactoryPreset (Preset&& preset);
    void setIsDirty (bool shouldBeDirty);

    int getIndexForPreset (const Preset& preset) const;
    const Preset* getPresetForIndex (int index) const;

    enum
    {
        factoryUserNumIDs = 100,
        userUserIDStart = 10000,
    };

    std::unordered_map<int, Preset> presetMap;
    std::unordered_map<juce::String, int> userIDMap;

    juce::ListenerList<Listener> listeners;
    bool isDirty = false;

    const Preset* currentPreset = nullptr;

    juce::String userPresetConfigPath;

    std::unique_ptr<Preset> statePreset;
    static const juce::Identifier presetDirtyTag;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetManager)
};

} // namespace chowdsp
