#pragma once

// - Save user preset
// - Load factory presets
// - Load user presets
// - Mark preset as "dirty"
// - Save preset name and "dirty" state

namespace chowdsp
{
class PresetManager
{
public:
    PresetManager (juce::AudioProcessor* p, juce::AudioProcessorValueTreeState& vts);

    void loadPresetFromIdx (int index);

    void saveUserPreset (const juce::File& file);

protected:
    virtual std::unique_ptr<juce::XmlElement> savePresetState();
    virtual void loadPresetState (juce::XmlElement* xml);

    juce::AudioProcessor* processor;
    juce::AudioProcessorValueTreeState& vts;

private:
    void loadPreset (const Preset& preset);

    enum
    {
        factoryUserNumIDs = 100,
        userUserIDStart = 10000,
    };

    std::unordered_map<int, Preset> presetMap;
    std::unordered_map<juce::String, int> userIDMap;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetManager)
};

} // namespace chowdsp
