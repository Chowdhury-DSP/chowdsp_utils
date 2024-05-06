namespace chowdsp
{
ChowLNF::ChowLNF()
{
    roboto = juce::Typeface::createSystemTypefaceFor (chowdsp_BinaryData::RobotoCondensedRegular_ttf,
                                                      chowdsp_BinaryData::RobotoCondensedRegular_ttfSize);

    robotoBold = juce::Typeface::createSystemTypefaceFor (chowdsp_BinaryData::RobotoCondensedBold_ttf,
                                                          chowdsp_BinaryData::RobotoCondensedBold_ttfSize);
}

juce::Typeface::Ptr ChowLNF::getTypefaceForFont (const juce::Font& font)
{
    return font.isBold() ? robotoBold : roboto;
}

void ChowLNF::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
{
    int diameter = (width > height) ? height : width;
    if (diameter < 16)
        return;

    juce::Point centre ((float) x + std::floor ((float) width * 0.5f + 0.5f), (float) y + std::floor ((float) height * 0.5f + 0.5f));
    diameter -= (diameter % 2) ? 9 : 8;
    float radius = (float) diameter * 0.5f;
    x = int (centre.x - radius);
    y = int (centre.y - radius);

    const auto bounds = juce::Rectangle<int> (x, y, diameter, diameter).toFloat();

    auto b = pointer->getBounds().toFloat();
    pointer->setTransform (juce::AffineTransform::rotation (juce::MathConstants<float>::twoPi * ((sliderPos - 0.5f) * 300.0f / 360.0f),
                                                            b.getCentreX(),
                                                            b.getCentreY()));

    const auto alphaMult = slider.isEnabled() ? 1.0f : 0.4f;
    auto knobBounds = (bounds * 0.75f).withCentre (centre);
    knob->drawWithin (g, knobBounds, juce::RectanglePlacement::stretchToFit, alphaMult);
    pointer->drawWithin (g, knobBounds, juce::RectanglePlacement::stretchToFit, alphaMult);

    const auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    constexpr float arcFactor = 0.9f;

    juce::Path valueArc;
    valueArc.addPieSegment (bounds, rotaryStartAngle, rotaryEndAngle, arcFactor);
    g.setColour (slider.findColour (juce::Slider::trackColourId).withMultipliedAlpha (alphaMult));
    g.fillPath (valueArc);
    valueArc.clear();

    valueArc.addPieSegment (bounds, rotaryStartAngle, toAngle, arcFactor);
    g.setColour (slider.findColour (juce::Slider::thumbColourId).withMultipliedAlpha (alphaMult));
    g.fillPath (valueArc);
}

void ChowLNF::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto fontSize = juce::jmin (15.0f, (float) button.getHeight() * 0.75f);
    auto tickWidth = fontSize * 1.1f;

    drawTickBox (g, button, 4.0f, ((float) button.getHeight() - tickWidth) * 0.5f, tickWidth, tickWidth, button.getToggleState(), button.isEnabled(), shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

    g.setColour (button.findColour (juce::ToggleButton::textColourId));
#if JUCE_VERSION < 0x080000
    g.setFont (juce::Font (fontSize).boldened());
#else
    g.setFont (juce::Font { juce::FontOptions { fontSize, juce::Font::bold } });
#endif

    if (! button.isEnabled())
        g.setOpacity (0.5f);

    g.drawFittedText (button.getButtonText(),
                      button.getLocalBounds().withTrimmedLeft (juce::roundToInt (tickWidth) + 10).withTrimmedRight (2),
                      juce::Justification::centredLeft,
                      10);
}

void ChowLNF::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    juce::LookAndFeel_V4::positionComboBoxText (box, label);
    label.setFont (label.getFont().boldened());
    label.setJustificationType (juce::Justification::centred);
}

void ChowLNF::createTabTextLayout (const juce::TabBarButton& button, float length, float depth, juce::Colour colour, juce::TextLayout& textLayout)
{
#if JUCE_VERSION < 0x080000
    juce::Font font (depth * 0.45f, juce::Font::bold);
#else
    juce::Font font { juce::FontOptions { depth * 0.45f, juce::Font::bold } };
#endif
    font.setUnderline (button.hasKeyboardFocus (false));

    juce::AttributedString s;
    s.setJustification (juce::Justification::centred);
    s.append (button.getButtonText().trim(), font, colour);

    textLayout.createLayout (s, length);
}

void ChowLNF::drawTabButton (juce::TabBarButton& button, juce::Graphics& g, bool /*isMouseOver*/, bool /*isMouseDown*/)
{
    const juce::Rectangle<int> activeArea (button.getActiveArea());

    const juce::TabbedButtonBar::Orientation o = button.getTabbedButtonBar().getOrientation();

    const juce::Colour bkg (button.getTabBackgroundColour());

    if (button.getToggleState())
    {
        g.setColour (bkg);
    }
    else
    {
        juce::Point<int> p1, p2;

        switch (o)
        {
            case juce::TabbedButtonBar::TabsAtBottom:
                p1 = activeArea.getBottomLeft();
                p2 = activeArea.getTopLeft();
                break;
            case juce::TabbedButtonBar::TabsAtTop:
                p1 = activeArea.getTopLeft();
                p2 = activeArea.getBottomLeft();
                break;
            case juce::TabbedButtonBar::TabsAtRight:
                p1 = activeArea.getTopRight();
                p2 = activeArea.getTopLeft();
                break;
            case juce::TabbedButtonBar::TabsAtLeft:
                p1 = activeArea.getTopLeft();
                p2 = activeArea.getTopRight();
                break;
            default:
                jassertfalse;
                break;
        }

        g.setGradientFill (juce::ColourGradient (bkg.brighter (0.2f), p1.toFloat(), bkg.darker (0.1f), p2.toFloat(), false));
    }

    g.fillRect (activeArea);

    g.setColour (button.findColour (juce::TabbedButtonBar::tabOutlineColourId));

    juce::Rectangle<int> r (activeArea);
    g.fillRect (r.removeFromBottom (1));

    const float alpha = 0.6f;
    juce::Colour col (bkg.contrasting().withMultipliedAlpha (alpha));

    if (button.isFrontTab())
        col = juce::Colours::white;

    const juce::Rectangle<float> area (button.getTextArea().toFloat());

    float length = area.getWidth();
    float depth = area.getHeight();

    if (button.getTabbedButtonBar().isVertical())
        std::swap (length, depth);

    juce::TextLayout textLayout;
    createTabTextLayout (button, length, depth, col, textLayout);

    juce::AffineTransform t;

    switch (o)
    {
        case juce::TabbedButtonBar::TabsAtLeft:
            t = t.rotated (juce::MathConstants<float>::pi * -0.5f).translated (area.getX(), area.getBottom());
            break;
        case juce::TabbedButtonBar::TabsAtRight:
            t = t.rotated (juce::MathConstants<float>::pi * 0.5f).translated (area.getRight(), area.getY());
            break;
        case juce::TabbedButtonBar::TabsAtTop:
        case juce::TabbedButtonBar::TabsAtBottom:
            t = t.translated (area.getX(), area.getY());
            break;
        default:
            jassertfalse;
            break;
    }

    g.addTransform (t);
    textLayout.draw (g, juce::Rectangle<float> (length, depth));
}

void ChowLNF::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/, const juce::Slider::SliderStyle, juce::Slider& slider)
{
    auto trackWidth = juce::jmin (6.0f, slider.isHorizontal() ? (float) height * 0.25f : (float) width * 0.25f);

    juce::Point startPoint (slider.isHorizontal() ? (float) x : (float) x + (float) width * 0.5f,
                            slider.isHorizontal() ? (float) y + (float) height * 0.5f : (float) (height + y));

    juce::Point endPoint (slider.isHorizontal() ? (float) (width + x) : startPoint.x,
                          slider.isHorizontal() ? startPoint.y : (float) y);

    juce::Path backgroundTrack;
    backgroundTrack.startNewSubPath (startPoint);
    backgroundTrack.lineTo (endPoint);

    const auto alphaMult = slider.isEnabled() ? 1.0f : 0.4f;
    g.setColour (slider.findColour (juce::Slider::backgroundColourId).withAlpha (alphaMult));
    g.strokePath (backgroundTrack, { trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });

    juce::Path valueTrack;
    juce::Point<float> minPoint, maxPoint;

    {
        auto kx = slider.isHorizontal() ? sliderPos : ((float) x + (float) width * 0.5f);
        auto ky = slider.isHorizontal() ? ((float) y + (float) height * 0.5f) : sliderPos;

        minPoint = startPoint;
        maxPoint = { kx, ky };
    }

    auto thumbWidth = getSliderThumbRadius (slider);

    valueTrack.startNewSubPath (minPoint);
    valueTrack.lineTo (maxPoint);
    g.setColour (slider.findColour (juce::Slider::thumbColourId).withAlpha (alphaMult));
    g.strokePath (valueTrack, { trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });

    auto thumbRect = juce::Rectangle<float> (static_cast<float> (thumbWidth),
                                             static_cast<float> (thumbWidth))
                         .withCentre (maxPoint);
    knob->drawWithin (g, thumbRect, juce::RectanglePlacement::stretchToFit, alphaMult);
}

juce::Slider::SliderLayout ChowLNF::getSliderLayout (juce::Slider& slider)
{
    auto layout = juce::LookAndFeel_V4::getSliderLayout (slider);

    auto style = slider.getSliderStyle();
    if (style == juce::Slider::LinearHorizontal)
        layout.textBoxBounds = layout.textBoxBounds.withX (layout.sliderBounds.getX());

    return layout;
}

juce::Label* ChowLNF::createSliderTextBox (juce::Slider& slider)
{
    auto l = juce::LookAndFeel_V4::createSliderTextBox (slider);

    auto style = slider.getSliderStyle();
    if (style == juce::Slider::LinearHorizontal)
        l->setJustificationType (juce::Justification::left);

#if JUCE_VERSION < 0x080000
    l->setFont ((float) slider.getTextBoxHeight());
#else
    l->setFont (juce::FontOptions { (float) slider.getTextBoxHeight() });
#endif

    return l;
}

juce::Component* ChowLNF::getParentComponentForMenuOptions (const juce::PopupMenu::Options& options) // NOSONAR
{
#if JUCE_IOS
    if (juce::PluginHostType::getPluginLoadedAs() == juce::AudioProcessor::wrapperType_AudioUnitv3)
    {
        if (options.getParentComponent() == nullptr && options.getTargetComponent() != nullptr)
            return options.getTargetComponent()->getTopLevelComponent();
    }
#endif
    return juce::LookAndFeel_V2::getParentComponentForMenuOptions (options);
}

} // namespace chowdsp
