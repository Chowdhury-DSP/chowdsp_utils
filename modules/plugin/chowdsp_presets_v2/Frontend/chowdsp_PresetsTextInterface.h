#pragma once

namespace chowdsp::presets::frontend
{
/** Interface for getting the text associated with the preset manager's state. */
class TextInterface
{
public:
    explicit TextInterface (PresetManager& manager);

    /** Returns the text associated with the current preset. */
    [[nodiscard]] juce::String getPresetText() const noexcept { return presetText; }

    /** Called whenever the preset text is changed. */
    Broadcaster<void (const juce::String&)> presetTextChangedBroadcaster;

private:
    void updateText();

    PresetManager& presetManager;
    juce::String presetText {};

    ScopedCallbackList listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextInterface)
};
} // namespace chowdsp::presets::frontend
