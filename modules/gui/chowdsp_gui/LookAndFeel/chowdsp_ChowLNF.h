#pragma once

#include <memory>

namespace chowdsp
{
/** LookAndFeel used by chowdsp plugins */
class ChowLNF : public juce::LookAndFeel_V4
{
public:
    ChowLNF();

    /** Returns Roboto or RobotoBold for all fonts */
    juce::Typeface::Ptr getTypefaceForFont (const juce::Font&) override;

    /** Draws a chowdsp rotary slider */
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override;

    /** Draws a chowdsp toggle button */
    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    /** Positions ComboBox text to leave room for Label */
    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override;

    /** Creates tabbed layout for tabbed components */
    static void createTabTextLayout (const juce::TabBarButton& button, float length, float depth, juce::Colour colour, juce::TextLayout& textLayout);

    /** Draws a tabbed button */
    void drawTabButton (juce::TabBarButton& button, juce::Graphics& g, bool isMouseOver, bool isMouseDown) override;

    /** Draws a chowdsp linear slider */
    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style, juce::Slider& slider) override;

    /** Returns the slider layout for a slider */
    juce::Slider::SliderLayout getSliderLayout (juce::Slider& slider) override;

    /** Creates a text box for a slider */
    juce::Label* createSliderTextBox (juce::Slider& slider) override;

    /** Finds the proper parent component for a popup menu with these options */
    juce::Component* getParentComponentForMenuOptions (const juce::PopupMenu::Options& options) override;

protected:
    std::unique_ptr<juce::Drawable> knob = juce::Drawable::createFromImageData (chowdsp_BinaryData::knob_svg, chowdsp_BinaryData::knob_svgSize);
    std::unique_ptr<juce::Drawable> pointer = juce::Drawable::createFromImageData (chowdsp_BinaryData::pointer_svg, chowdsp_BinaryData::pointer_svgSize);

    juce::Typeface::Ptr roboto;
    juce::Typeface::Ptr robotoBold;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChowLNF)
};

} // namespace chowdsp
