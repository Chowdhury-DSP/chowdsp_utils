#pragma once

namespace chowdsp
{

class TitleComp : public juce::Component,
                  public juce::SettableTooltipClient
{
public:
    TitleComp();

    enum ColourIDs
    {
        text1ColourID,
        text2ColourID,
    };

    void paint (juce::Graphics& g) override;
    void setStrings (juce::String newTitle, juce::String newSubtitle, float font);

private:
    juce::String title;
    juce::String subtitle;
    float font = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TitleComp)
};

class TitleItem : public foleys::GuiItem
{
public:
    FOLEYS_DECLARE_GUI_FACTORY (TitleItem)

    static const juce::Identifier title;
    static const juce::Identifier subtitle;
    static const juce::Identifier font;

    TitleItem (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node);

    void update() override;
    std::vector<foleys::SettableProperty> getSettableProperties() const override;

    juce::Component* getWrappedComponent() override { return &comp; }

private:
    TitleComp comp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TitleItem)
};

} // chowdsp
