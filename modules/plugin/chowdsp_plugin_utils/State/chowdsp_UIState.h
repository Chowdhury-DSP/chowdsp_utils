#pragma once

namespace chowdsp
{
/** Manages the state of the plugin UI */
class UIState : private juce::ComponentListener
{
public:
    /** Constructor */
    explicit UIState (juce::AudioProcessorValueTreeState& vts, int defaultWidth = 400, int defaultHeight = 400);

    /** Attaches the state to the plugin editor */
    void attachToComponent (juce::AudioProcessorEditor& comp);

    /** Returns the previous editor size */
    [[nodiscard]] juce::Rectangle<int> getLastEditorSize() const;

private:
    void componentBeingDeleted (juce::Component& component) override;
    void componentMovedOrResized (juce::Component& component, bool wasMoved, bool wasResized) override;
    void setLastEditorSize (const juce::Rectangle<int>& bounds);

    juce::AudioProcessorValueTreeState& vts;

    const int defaultWidth;
    const int defaultHeight;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIState)
};
} // namespace chowdsp
