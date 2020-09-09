namespace chowdsp
{

TooltipComponent::TooltipComponent()
{
    setColour (backgroundColourID, juce::Colours::transparentBlack);
    setColour (textColourID, juce::Colours::lightgrey);
    setColour (nameColourID, juce::Colours::white);

    showTip.store (false);
    startTimer (123);
}

void TooltipComponent::paint (juce::Graphics& g)
{
    g.fillAll (findColour (backgroundColourID));

    if (showTip.load())
    {
        auto b = getLocalBounds();

        g.setFont (juce::Font (17.0f).boldened());
        if (name.isNotEmpty())
        {
            g.setColour (findColour (nameColourID));
            g.drawFittedText (name + ":", b, juce::Justification::topLeft, 1);
        }

        auto whitespace = juce::String();
        auto font = g.getCurrentFont();
        while (font.getStringWidth(whitespace) < font.getStringWidth (name + ": "))
            whitespace += " ";

        g.setColour (findColour (textColourID));
        g.drawMultiLineText (whitespace + tip, b.getX(),
            b.getY() + (int) font.getHeight() - 3, b.getWidth(), juce::Justification::topLeft);
    }
}

juce::String TooltipComponent::getTipFor (juce::Component& c)
{
    if (juce::Process::isForegroundProcess()
         && ! juce::ModifierKeys::currentModifiers.isAnyMouseButtonDown())
    {
        if (auto* ttc = dynamic_cast<juce::TooltipClient*> (&c))
            if (! c.isCurrentlyBlockedByAnotherModalComponent())
                return ttc->getTooltip();
    }

    return {};
}

void TooltipComponent::timerCallback()
{
    auto& desktop = juce::Desktop::getInstance();
    auto mouseSource = desktop.getMainMouseSource();

    auto* newComp = mouseSource.isTouch() ? nullptr : mouseSource.getComponentUnderMouse();

    bool needsRepaint = false;
    if (newComp != nullptr)
    {
        auto newTip = getTipFor (*newComp);
        needsRepaint = newTip != tip;

        tip = newTip;
        name = newComp->getName();   

        if (! showTip.load())
        {
            showTip.store (true);
            needsRepaint = true;
        }
    }
    else
    {
        if (showTip.load())
        {
            showTip.store (false);
            needsRepaint = true;
        }
    }

    if (needsRepaint)
        repaint();
}

} // chowdsp
