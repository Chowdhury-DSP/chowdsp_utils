#pragma once

namespace chowdsp
{
/**
 * An object of this class maintains a connection between a juce::Slider and a plug-in parameter.
 *
 * During the lifetime of this object it keeps the two things in sync, making it easy to connect a slider
 * to a parameter. When this object is deleted, the connection is broken. Make sure that your parameter and
 * Slider are not deleted before this object!
 */
class SliderAttachment : private juce::Slider::Listener
{
public:
    /** Creates an attachment for a given parameter, using the undo manager from the plugin state. */
    SliderAttachment (FloatParameter& param,
                      PluginState& pluginState,
                      juce::Slider& paramSlider);

    /** Creates an attachment for a given parameter. */
    SliderAttachment (FloatParameter& param,
                      ParameterListeners& listeners,
                      juce::Slider& paramSlider,
                      juce::UndoManager* undoManager);

    SliderAttachment() = default;
    SliderAttachment (SliderAttachment&&) noexcept = default;
    SliderAttachment& operator= (SliderAttachment&&) noexcept = default;

    ~SliderAttachment() override;

    /** Sets the initial value of the slider */
    void setValue (float newValue);

    /** Returns the attached parameter */
    [[nodiscard]] const FloatParameter* getParameter() const { return attachment.param; }

private:
    void sliderValueChanged (juce::Slider*) override;
    void sliderDragStarted (juce::Slider*) override;
    void sliderDragEnded (juce::Slider*) override;

    juce::Slider* slider = nullptr;
    ParameterAttachment<FloatParameter,
                        ParameterAttachmentHelpers::SetValueCallback<SliderAttachment>>
        attachment;
    juce::UndoManager* um = nullptr;

    bool skipSliderChangedCallback = false;
    float valueAtStartOfGesture = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SliderAttachment)
};
} // namespace chowdsp
