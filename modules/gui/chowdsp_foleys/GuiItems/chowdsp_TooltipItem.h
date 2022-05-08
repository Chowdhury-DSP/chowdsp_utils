#pragma once

namespace chowdsp
{
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
} // namespace chowdsp
