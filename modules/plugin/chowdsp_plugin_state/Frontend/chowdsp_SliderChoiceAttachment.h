#pragma once

namespace chowdsp
{
class SliderChoiceAttachment : private juce::Slider::Listener
{
public:
    /** Creates an attachment for a given parameter, using the undo manager from the plugin state. */
    SliderChoiceAttachment (ChoiceParameter& param,
                            PluginState& pluginState,
                            juce::Slider& paramSlider);

    /** Creates an attachment for a given parameter. */
    SliderChoiceAttachment (ChoiceParameter& param,
                            ParameterListeners& listeners,
                            juce::Slider& paramSlider,
                            juce::UndoManager* undoManager);

    SliderChoiceAttachment() = default;
    SliderChoiceAttachment (SliderChoiceAttachment&&) noexcept = default;
    SliderChoiceAttachment& operator= (SliderChoiceAttachment&&) noexcept = default;

    ~SliderChoiceAttachment() override;

    /** Sets the initial value of the slider */
    void setValue (int newValue);

    /** Returns the attached parameter */
    [[nodiscard]] const ChoiceParameter* getParameter() const { return attachment.param; }

private:
    void sliderValueChanged (juce::Slider*) override;
    void sliderDragStarted (juce::Slider*) override;
    void sliderDragEnded (juce::Slider*) override;

    juce::Slider* slider = nullptr;
    ParameterAttachment<ChoiceParameter,
                        ParameterAttachmentHelpers::SetValueCallback<SliderChoiceAttachment>>
        attachment;
    juce::UndoManager* um = nullptr;

    bool skipSliderChangedCallback = false;
    int valueAtStartOfGesture = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SliderChoiceAttachment)
};
} // namespace chowdsp
