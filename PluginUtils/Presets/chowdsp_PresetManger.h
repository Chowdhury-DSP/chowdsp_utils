#pragma once

// - Save user preset
// - Load factory presets
// - Load user presets
// - Mark preset as "dirty"
// - Save preset name and "dirty" state

namespace chowdsp
{
class PresetManager : private juce::AudioProcessorValueTreeState::Listener
{
public:
    PresetManager (juce::AudioProcessorValueTreeState& vts);
    ~PresetManager();

    void loadPresetFromIdx (int index);

    void saveUserPreset (const juce::File& file);

    void parameterChanged (const juce::String& parameterID, float newValue) override;
    bool getIsDirty() const noexcept { return isDirty; }

    struct Listener
    {
        virtual ~Listener() = default;
        virtual void presetListUpdated() {}
        virtual void presetDirtyStatusChanged() {}
    };

    void addListener (Listener* l) { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

protected:
    virtual std::unique_ptr<juce::XmlElement> savePresetState();
    virtual void loadPresetState (juce::XmlElement* xml);

    juce::AudioProcessorValueTreeState& vts;
    juce::AudioProcessor& processor;

private:
    void loadPreset (const Preset& preset);
    void setIsDirty (bool shouldBeDirty);

    enum
    {
        factoryUserNumIDs = 100,
        userUserIDStart = 10000,
    };

    std::unordered_map<int, Preset> presetMap;
    std::unordered_map<juce::String, int> userIDMap;

    juce::ListenerList<Listener> listeners;
    bool isDirty = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetManager)
};

} // namespace chowdsp
