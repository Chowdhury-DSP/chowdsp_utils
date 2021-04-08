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

void TitleComp::setStrings (juce::String newTitle, juce::String newSubtitle, float newFont)
{
    font = newFont == 0.0f ? (float) getHeight() : newFont;

    title = newTitle;
    subtitle = newSubtitle;
    repaint();
}

#if CHOWDSP_USE_FOLEYS_CLASSES
//======================================================================
TitleItem::TitleItem (foleys::MagicGUIBuilder& builder, const juce::ValueTree& node) : foleys::GuiItem (builder, node)
{
    setColourTranslation ({
        { "text1", TitleComp::text1ColourID },
        { "text2", TitleComp::text2ColourID },
    });

    addAndMakeVisible (comp);
}

void TitleItem::update()
{
    auto titleString = magicBuilder.getStyleProperty (title, configNode).toString();
    auto subtitleString = magicBuilder.getStyleProperty (subtitle, configNode).toString();
    auto fontVal = (float) magicBuilder.getStyleProperty (font, configNode);

    comp.setStrings (titleString, subtitleString, fontVal);
}

std::vector<foleys::SettableProperty> TitleItem::getSettableProperties() const
{
    std::vector<foleys::SettableProperty> props;
    props.push_back ({ configNode, title, foleys::SettableProperty::Text, {}, {} });
    props.push_back ({ configNode, subtitle, foleys::SettableProperty::Text, {}, {} });
    props.push_back ({ configNode, font, foleys::SettableProperty::Number, 0.0f, {} });
    return props;
}

const juce::Identifier TitleItem::title { "title" };
const juce::Identifier TitleItem::subtitle { "subtitle" };
const juce::Identifier TitleItem::font { "font" };

#endif

} // namespace chowdsp
