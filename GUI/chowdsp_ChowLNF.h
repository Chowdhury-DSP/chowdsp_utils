#pragma once

#include <memory>

namespace chowdsp
{

class ChowLNF : public juce::LookAndFeel_V4
{
public:
    ChowLNF();

    juce::Typeface::Ptr getTypefaceForFont(const juce::Font&) override;

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider& slider) override;

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override;

    void createTabTextLayout (const juce::TabBarButton& button, float length, float depth,
                              juce::Colour colour, juce::TextLayout& textLayout);
    void drawTabButton (juce::TabBarButton& button, juce::Graphics& g, bool isMouseOver, bool isMouseDown) override;

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const juce::Slider::SliderStyle style, juce::Slider& slider) override;

    juce::Slider::SliderLayout getSliderLayout (juce::Slider& slider) override;
    juce::Label* createSliderTextBox (juce::Slider& slider) override;

private:
    std::unique_ptr<juce::Drawable> knob = juce::Drawable::createFromImageData (chowdsp_BinaryData::knob_svg, chowdsp_BinaryData::knob_svgSize);
    std::unique_ptr<juce::Drawable> pointer = juce::Drawable::createFromImageData (chowdsp_BinaryData::pointer_svg, chowdsp_BinaryData::pointer_svgSize);

    juce::Typeface::Ptr roboto;
    juce::Typeface::Ptr robotoBold;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChowLNF)
};

} // chowdsp
