namespace chowdsp
{
template <typename ProcType, typename InfoProvider>
InfoComp<ProcType, InfoProvider>::InfoComp (const ProcType& processor) : proc (processor)
{
    setColour (text1ColourID, juce::Colours::grey);
    setColour (text2ColourID, juce::Colours::white);

    addAndMakeVisible (linkButton);
}

template <typename ProcType, typename InfoProvider>
void InfoComp<ProcType, InfoProvider>::paint (juce::Graphics& g)
{
    auto width = (float) getWidth() - 10.0f;

    using namespace SystemInfo;
    auto platformStr = InfoProvider::getPlatformString() + ", ";
    auto typeStr = InfoProvider::getWrapperTypeString (proc) + ", ";
    auto versionStr = InfoProvider::getVersionString() + " ";
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

    auto drawText = [&font, &g, &b] (const juce::String& text)
    {
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

    if constexpr (InfoProvider::showDebugFlag())
    {
        g.fillAll (juce::Colours::red);
        g.setColour (juce::Colours::yellow);
        g.setFont (juce::Font ((float) getHeight()).boldened());
        g.drawFittedText ("DEBUG", getLocalBounds(), juce::Justification::centred, 1);
        linkButton.setVisible (false);
    }
}

template <typename ProcType, typename InfoProvider>
void InfoComp<ProcType, InfoProvider>::resized()
{
    linkButton.setBounds (linkX, 0, 100, getHeight());
}

} // namespace chowdsp
