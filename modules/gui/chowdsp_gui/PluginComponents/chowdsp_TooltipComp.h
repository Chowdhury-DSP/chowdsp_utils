#pragma once

namespace chowdsp
{
/** Component to display the name and tooltip of whatever component is under the mouse. */
class TooltipComponent : public juce::Component,
                         private juce::Timer
{
public:
    TooltipComponent();

    enum ColourIDs
    {
        backgroundColourID, /**< Background colour for the component */
        textColourID, /**< Colour to use for the tooltip text */
        nameColourID, /**< Colour to use for the tooltip name */
    };

    void paint (juce::Graphics& g) override;
    void timerCallback() override;
    static void getTipFor (juce::Component& c, juce::String& newTip, juce::String& newName);

protected:
    juce::String name, tip;
    std::atomic_bool showTip { false };

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TooltipComponent)
};
} // namespace chowdsp
