#pragma once

namespace chowdsp::presets::frontend
{
/** Interface for navigating forwards and backwards through the preset tree. */
class NextPrevious
{
public:
    explicit NextPrevious (PresetManager& manager);

    /** Use this method to set the desired behaviour when reaching the end of the presets list. */
    void setShouldWrapAtEndOfList (bool shouldWrap) { wrapAtEndOfTree = shouldWrap; }

    /** Return true if NextPrevious will "wrap" when it reaches the end of the preset list. */
    [[nodiscard]] bool willWrapAtEndOFList() const noexcept { return wrapAtEndOfTree; }

    /** Attempts to load the next preset in the list and returns true if successful. */
    bool goToNextPreset();

    /** Attempts to load the previous preset in the list and returns true if successful. */
    bool goToPreviousPreset();

private:
    bool navigateThroughPresets (bool forward);

    PresetManager& presetManager;
    bool wrapAtEndOfTree = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NextPrevious)
};
} // namespace chowdsp::presets::frontend
