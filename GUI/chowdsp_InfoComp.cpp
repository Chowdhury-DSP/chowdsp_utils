namespace chowdsp
{
InfoComp::InfoComp (const juce::AudioProcessor::WrapperType pluginWrapperType) : wrapperType (pluginWrapperType),
#if defined JucePlugin_ManufacturerWebsite
                                                                                 linkButton (JucePlugin_Manufacturer, juce::URL (JucePlugin_ManufacturerWebsite))
#else
                                                                                 linkButton (JucePlugin_Manufacturer, juce::URL ("https://chowdsp.com"))
#endif
{
    setColour (text1ColourID, juce::Colours::grey);
    setColour (text2ColourID, juce::Colours::white);

    addAndMakeVisible (linkButton);
    linkButton.setFont (juce::Font (17.0f).boldened(), false, juce::Justification::left);
}

void InfoComp::paint (juce::Graphics& g)
{
    g.setFont (17.0f);
    auto font = g.getCurrentFont();
    auto b = getLocalBounds();

    auto drawText = [=, &g, &b] (const juce::String& text) {
        auto width = font.getStringWidth (text);
        g.drawFittedText (text, b.removeFromLeft (width), juce::Justification::left, 1);
    };

    auto typeStr = juce::String (juce::AudioProcessor::getWrapperTypeDescription (wrapperType));
    g.setColour (findColour (text1ColourID));
    drawText (typeStr + ", ");

    g.setColour (findColour (text2ColourID));
    drawText ("v" + juce::String (JucePlugin_VersionString) + " ");

    g.setColour (findColour (text1ColourID));
    drawText (juce::String ("~ DSP by "));

    linkX = b.getX() - 2;
    linkButton.setColour (juce::HyperlinkButton::ColourIds::textColourId, findColour (text2ColourID));
    resized();
}

void InfoComp::resized()
{
    linkButton.setBounds (linkX, 0, 100, getHeight());
}

} // namespace chowdsp
