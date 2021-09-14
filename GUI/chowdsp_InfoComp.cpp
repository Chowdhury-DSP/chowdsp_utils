namespace chowdsp
{
constexpr std::string_view getOSDescription()
{
#if JUCE_WIN
#if JUCE_64BIT
    return "Win64";
#elif JUCE_32BIT
    return "Win32";
#endif
#elif JUCE_MAC
    return "Mac";
#elif JUCE_IOS
    return "IOS";
#elif JUCE_LINUX
#if JUCE_64BIT
    return "Linux64";
#elif JUCE_32BIT
    return "Linux32";
#endif
#endif
}

constexpr std::string_view getProcArch()
{
#if JUCE_INTEL
    return "Intel";
#elif JUCE_ARM
    return "ARM";
#else
    return "N/A";
#endif
}

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
    linkButton.setFont (juce::Font (17.0f).boldened(), false, juce::Justification::left);
}

void InfoComp::paint (juce::Graphics& g)
{
    g.setFont (17.0f);
    auto font = g.getCurrentFont();
    auto b = getLocalBounds();

    auto drawText = [=, &g, &b] (const juce::String& text)
    {
        auto width = font.getStringWidth (text);
        g.drawFittedText (text, b.removeFromLeft (width), juce::Justification::left, 1);
    };

    auto platformStr = juce::String (getOSDescription().data()) + "-" + juce::String (getProcArch().data());
    auto typeStr = juce::String (juce::AudioProcessor::getWrapperTypeDescription (wrapperType));
    g.setColour (findColour (text1ColourID));
    drawText (platformStr + ", ");
    drawText (typeStr + ", ");

    g.setColour (findColour (text2ColourID));
#if defined JucePlugin_VersionString
    drawText ("v" + juce::String (JucePlugin_VersionString) + " ");
#else
    drawText ("No Version" + " ");
#endif

    g.setColour (findColour (text1ColourID));
    drawText (juce::String ("~ DSP by "));

    linkX = b.getX() - 2;
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
