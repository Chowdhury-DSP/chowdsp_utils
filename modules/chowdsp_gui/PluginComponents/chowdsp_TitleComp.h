#pragma once

namespace chowdsp
{
/** A simple component to dislay the title and subtitle of a plugin */
class TitleComp : public juce::Component,
                  public juce::SettableTooltipClient
{
public:
    TitleComp();

    enum ColourIDs
    {
        text1ColourID, /**< Colour to use for plugin title */
        text2ColourID, /**< Colour to use for plugin subtitle */
    };

    void paint (juce::Graphics& g) override;

    /** Sets the strings to use for the plugin title
     * 
     * @param newTitle      Main plugin title
     * @param newSubtitle   Secondary plugin title
     * @param font          Font size to use for drawing the title
    */
    void setStrings (juce::String newTitle, juce::String newSubtitle, float font);

private:
    juce::String title;
    juce::String subtitle;
    float font = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TitleComp)
};

#if CHOWDSP_USE_FOLEYS_CLASSES
/** Foley's GUI wrapper for TitleComp */
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TitleItem)
};
#endif

} // namespace chowdsp
