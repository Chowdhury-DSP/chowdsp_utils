#include "chowdsp_TooltipComp.h"

namespace chowdsp
{
TooltipComponent::TooltipComponent()
{
    setColour (backgroundColourID, juce::Colours::transparentBlack);
    setColour (textColourID, juce::Colours::lightgrey);
    setColour (nameColourID, juce::Colours::white);

    startTimer (123);
}

void TooltipComponent::paint (juce::Graphics& g)
{
    g.fillAll (findColour (backgroundColourID));

    if (showTip.load())
    {
        auto b = getLocalBounds();

#if JUCE_VERSION < 0x080000
        g.setFont (juce::Font (17.0f).boldened());
#else
        g.setFont (juce::Font { juce::FontOptions { 17.0f, juce::Font::bold } });
#endif
        if (name.isNotEmpty())
        {
            g.setColour (findColour (nameColourID));
            g.drawFittedText (name + ":", b, juce::Justification::topLeft, 1);
        }

        auto whitespace = juce::String();
        auto font = g.getCurrentFont();
        while (font.getStringWidth (whitespace) < font.getStringWidth (name + ": "))
            whitespace += " ";

        g.setColour (findColour (textColourID));
        g.drawMultiLineText (whitespace + tip, b.getX(), b.getY() + (int) font.getHeight() - 3, b.getWidth(), juce::Justification::topLeft);
    }
}

void TooltipComponent::getTipFor (juce::Component& c, juce::String& newTip, juce::String& newName)
{
    if (juce::Process::isForegroundProcess()
        && ! juce::ModifierKeys::currentModifiers.isAnyMouseButtonDown())
    {
        if (auto* ttc = dynamic_cast<juce::TooltipClient*> (&c))
        {
            if (! c.isCurrentlyBlockedByAnotherModalComponent())
            {
                newTip = ttc->getTooltip();
                newName = c.getName();
            }
        }
    }
}

void TooltipComponent::timerCallback()
{
    const auto& desktop = juce::Desktop::getInstance();
    auto mouseSource = desktop.getMainMouseSource();

    auto* newComp = mouseSource.isTouch() ? nullptr : mouseSource.getComponentUnderMouse();

    bool needsRepaint = false;
    if (newComp != nullptr)
    {
        juce::String newTip, newName;
        getTipFor (*newComp, newTip, newName);
        needsRepaint = newTip != tip;

        if (newTip.isNotEmpty() && newName.isEmpty())
        {
            if (auto parent = newComp->getParentComponent())
                newName = parent->getName();
        }

        tip = newTip;
        name = newName;

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

} // namespace chowdsp
