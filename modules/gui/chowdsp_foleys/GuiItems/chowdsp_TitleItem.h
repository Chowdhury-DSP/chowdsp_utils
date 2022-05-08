#pragma once

namespace chowdsp
{
/** Foley's GUI wrapper for TitleComp */
class TitleItem : public foleys::GuiItem
{
public:
    FOLEYS_DECLARE_GUI_FACTORY (TitleItem)

    static inline const juce::Identifier title { "title" };
    static inline const juce::Identifier subtitle { "subtitle" };
    static inline const juce::Identifier font { "font" };

    TitleItem (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node) : foleys::GuiItem (builder, node)
    {
        setColourTranslation ({
            { "text1", TitleComp::text1ColourID },
            { "text2", TitleComp::text2ColourID },
        });

        addAndMakeVisible (comp);
    }

    void update() override
    {
        auto titleString = magicBuilder.getStyleProperty (title, configNode).toString();
        auto subtitleString = magicBuilder.getStyleProperty (subtitle, configNode).toString();
        auto fontVal = (float) magicBuilder.getStyleProperty (font, configNode);

        comp.setStrings (titleString, subtitleString, fontVal);
    }

    [[nodiscard]] std::vector<foleys::SettableProperty> getSettableProperties() const override
    {
        std::vector<foleys::SettableProperty> props;
        props.push_back ({ configNode, title, foleys::SettableProperty::Text, {}, {} });
        props.push_back ({ configNode, subtitle, foleys::SettableProperty::Text, {}, {} });
        props.push_back ({ configNode, font, foleys::SettableProperty::Number, 0.0f, {} });
        return props;
    }

    juce::Component* getWrappedComponent() override { return &comp; }

private:
    TitleComp comp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TitleItem)
};
} // namespace chowdsp
