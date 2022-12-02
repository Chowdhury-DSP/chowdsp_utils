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
template <typename PluginStateType>
class SliderAttachment : private juce::Slider::Listener
{
public:
    /** Creates an attachment for a given parameter path */
    SliderAttachment (const ParameterPath& paramPath,
                      PluginStateType& pluginState,
                      juce::Slider& paramSlider);

    /** Creates an attachment for a given parameter, using the undo manager from the plugin state */
    SliderAttachment (FloatParameter& param,
                      PluginStateType& pluginState,
                      juce::Slider& paramSlider);

    /** Creates an attachment for a given parameter, using a custom UndoManager */
    SliderAttachment (FloatParameter& param,
                      PluginStateType& pluginState,
                      juce::Slider& paramSlider,
                      juce::UndoManager* undoManager);

    ~SliderAttachment() override;

    /** Sets the initial value of the slider */
    void setValue (float newValue);

private:
    void sliderValueChanged (juce::Slider*) override;
    void sliderDragStarted (juce::Slider*) override;
    void sliderDragEnded (juce::Slider*) override;

    juce::Slider& slider;
    ParameterAttachment<FloatParameter,
                        PluginStateType,
                        ParameterAttachmentHelpers::SetValueCallback<SliderAttachment>>
        attachment;
    juce::UndoManager* um = nullptr;

    bool skipSliderChangedCallback = false;
    float valueAtStartOfGesture = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SliderAttachment)
};
} // namespace chowdsp

#include "chowdsp_SliderAttachment.cpp"
