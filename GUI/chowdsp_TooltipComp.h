#pragma once

namespace chowdsp
{

class TooltipComponent : public juce::Component,
                         private juce::Timer
{
public:
    TooltipComponent();

    enum ColourIDs
    {
        backgroundColourID,
        textColourID,
        nameColourID,
    };

    void paint (juce::Graphics& g) override;
    void timerCallback() override;
    juce::String getTipFor (juce::Component& c);

private:
    juce::String name, tip;
    std::atomic_bool showTip;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TooltipComponent)
};

class TooltipItem : public foleys::GuiItem
{
public:
    FOLEYS_DECLARE_GUI_FACTORY (TooltipItem)

    TooltipItem (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node) :
        foleys::GuiItem (builder, node)
    {
        setColourTranslation ({
            {"tooltip-background", TooltipComponent::backgroundColourID},
            {"tooltip-text",       TooltipComponent::textColourID},
            {"tooltip-name",       TooltipComponent::nameColourID},
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

} // chowdsp
