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
    void setStrings (const juce::String& newTitle, const juce::String& newSubtitle, float font);

private:
    juce::String title;
    juce::String subtitle;
    float font = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TitleComp)
};
} // namespace chowdsp
