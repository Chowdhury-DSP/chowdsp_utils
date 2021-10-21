namespace chowdsp
{
InfoComp::InfoComp (const juce::AudioProcessor::WrapperType pluginWrapperType) : wrapperType (pluginWrapperType),
#if defined JucePlugin_Manufacturer
#if defined JucePlugin_ManufacturerWebsite
                                                                                 linkButton (JucePlugin_Manufacturer, juce::URL (JucePlugin_ManufacturerWebsite))
#else
                                                                                 linkButton (JucePlugin_Manufacturer, juce::URL ("https://chowdsp.com"))
#endif
#else
                                                                                 linkButton ("Manu", juce::URL ("https://website.com"))
#endif
{
    setColour (text1ColourID, juce::Colours::grey);
    setColour (text2ColourID, juce::Colours::white);

    addAndMakeVisible (linkButton);
}

void InfoComp::paint (juce::Graphics& g)
{
    auto width = (float) getWidth() - 10.0f;

    using namespace SystemInfo;
    auto platformStr = juce::String (getOSDescription().data()) + "-" + juce::String (getProcArch().data()) + ", ";
    auto typeStr = juce::String (juce::AudioProcessor::getWrapperTypeDescription (wrapperType)) + ", ";
#if defined JucePlugin_VersionString
    auto versionStr = "v" + juce::String (JucePlugin_VersionString) + " ";
#else
    auto versionStr = "No Version" + " ";
#endif
    auto dspStr = juce::String ("~ DSP by ");
    auto totalStr = platformStr + typeStr + versionStr + dspStr + linkButton.getButtonText();

    const auto defaultFont = juce::jmin (20.0f, (float) getHeight());
    g.setFont (defaultFont);
    auto fw = g.getCurrentFont().getStringWidthFloat (totalStr);
    if (fw > width)
    {
        float s = width / fw;
        g.setFont (defaultFont * s);
    }

    auto font = g.getCurrentFont();
    auto b = getLocalBounds();

    auto drawText = [=, &g, &b] (const juce::String& text) {
        auto w = font.getStringWidth (text);
        g.drawFittedText (text, b.removeFromLeft (w), juce::Justification::left, 1);
    };

    g.setColour (findColour (text1ColourID));
    drawText (platformStr);
    drawText (typeStr);

    g.setColour (findColour (text2ColourID));
    drawText (versionStr);

    g.setColour (findColour (text1ColourID));
    drawText (juce::String ("~ DSP by "));

    linkX = b.getX() - 2;
    linkButton.setFont (font.boldened(), false, juce::Justification::left);
    linkButton.setColour (juce::HyperlinkButton::ColourIds::textColourId, findColour (text2ColourID));
    resized();

#if JUCE_DEBUG
    g.fillAll (juce::Colours::red);
    g.setColour (juce::Colours::yellow);
    g.setFont (juce::Font ((float) getHeight()).boldened());
    g.drawFittedText ("DEBUG", getLocalBounds(), juce::Justification::centred, 1);
    linkButton.setVisible (false);
#endif
}

void InfoComp::resized()
{
    linkButton.setBounds (linkX, 0, 100, getHeight());
}

} // namespace chowdsp
