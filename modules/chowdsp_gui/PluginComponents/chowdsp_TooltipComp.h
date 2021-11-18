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
    void getTipFor (juce::Component& c, juce::String& newTip, juce::String& newName);

protected:
    juce::String name, tip;
    std::atomic_bool showTip;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TooltipComponent)
};

#if CHOWDSP_USE_FOLEYS_CLASSES
/** Foley's GUI wrapper for TooltipComponent */
class TooltipItem : public foleys::GuiItem
{
public:
    FOLEYS_DECLARE_GUI_FACTORY (TooltipItem)

    TooltipItem (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node) : foleys::GuiItem (builder, node)
    {
        setColourTranslation ({
            { "tooltip-background", TooltipComponent::backgroundColourID },
            { "tooltip-text", TooltipComponent::textColourID },
            { "tooltip-name", TooltipComponent::nameColourID },
        });

        addAndMakeVisible (tooltipComp);
    }

    void update() override
    {
    }

    juce::Component* getWrappedComponent() override
    {
        return &tooltipComp;
    }

private:
    TooltipComponent tooltipComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TooltipItem)
};
#endif

} // namespace chowdsp
