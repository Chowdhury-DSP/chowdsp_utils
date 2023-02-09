#pragma once

namespace chowdsp::presets::frontend
{
/** Interface for interacting with presets from the system clipboard. */
class ClipboardInterface
{
public:
    explicit ClipboardInterface (PresetManager& manager);

    /** Copies the current preset to the system clipboard. */
    void copyCurrentPreset() const;

    /** Tries to load the system clipboard contents as a preset. */
    bool tryToPastePreset();

private:
    PresetManager& presetManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClipboardInterface)
};
} // namespace chowdsp::presets::frontend
