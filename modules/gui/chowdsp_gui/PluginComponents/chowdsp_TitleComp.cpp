#include "chowdsp_TitleComp.h"

namespace chowdsp
{
TitleComp::TitleComp()
{
    setColour (text1ColourID, juce::Colours::white);
    setColour (text2ColourID, juce::Colours::grey);
}

void TitleComp::paint (juce::Graphics& g)
{
    g.setFont (juce::Font (font).boldened());
    auto curFont = g.getCurrentFont();
    auto b = getLocalBounds();

    auto drawText = [=, &g, &b] (const juce::String& text) {
        auto width = curFont.getStringWidth (text);
        g.drawFittedText (text, b.removeFromLeft (width), juce::Justification::left, 1);
    };

    g.setColour (findColour (text1ColourID));
    drawText (title + " ");

    g.setColour (findColour (text2ColourID));
    drawText (subtitle);
}

void TitleComp::setStrings (const juce::String& newTitle, const juce::String& newSubtitle, float newFont)
{
    font = newFont == 0.0f ? (float) getHeight() : newFont;

    title = newTitle;
    subtitle = newSubtitle;
    repaint();
}
} // namespace chowdsp
