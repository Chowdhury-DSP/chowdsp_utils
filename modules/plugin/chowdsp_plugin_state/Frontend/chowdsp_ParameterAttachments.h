#pragma once

namespace chowdsp
{
template <typename ParamType, typename PluginStateType, typename Callback = std::function<void (ParameterTypeHelpers::ParameterElementType<ParamType>)>>
class ParameterAttachment
{
public:
    ParameterAttachment (ParamType& parameter,
                         PluginStateType& pluginState,
                         Callback&& callback,
                         juce::UndoManager* um = nullptr);

    using ParamElementType = ParameterTypeHelpers::ParameterElementType<ParamType>;

    /**
     * Begins a gesture on the managed parameter.
     * Call this when the UI is about to begin a continuous interaction,
     * like when the mouse button is pressed on a slider.
     */
    void beginGesture();

    /**
     * Ends a gesture on the managed parameter.
     * Call this when the UI has finished a continuous interaction,
     * like when the mouse button is released on a slider.
     */
    void endGesture();

    /**
     * Triggers a full gesture message on the managed parameter.
     * Call this in the listener callback of the UI control in response
     * to a one-off change in the UI like a button-press.
     */
    void setValueAsCompleteGesture (ParamElementType newValue);

    /**
     * Updates the parameter value during a gesture.
     * Call this during a continuous interaction, like a slider value
     * changed callback.
     */
    void setValueAsPartOfGesture (ParamElementType newValue);

private:
    template <typename Func>
    void callIfParameterValueChanged (ParamElementType newValue, Func&& func);

    ParamType& param;
    chowdsp::ScopedCallback valueChangedCallback;
    juce::UndoManager* undoManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterAttachment)
};

template <typename Attachment>
struct SetValueCallback
{
    explicit SetValueCallback (Attachment& a) : attach (a) {}

    template <typename T>
    void operator() (T val)
    {
        attach.setValue (val);
    }

    Attachment& attach;
};

template <typename PluginStateType>
class SliderAttachment : private juce::Slider::Listener
{
public:
    SliderAttachment (FloatParameter& param,
                      PluginStateType& pluginState,
                      juce::Slider& paramSlider,
                      juce::UndoManager* undoManager = nullptr);

    ~SliderAttachment() override;

    void setValue (float newValue);

private:
    void sliderValueChanged (juce::Slider*) override;
    void sliderDragStarted (juce::Slider*) override;
    void sliderDragEnded (juce::Slider*) override;

    juce::Slider& slider;
    ParameterAttachment<FloatParameter,
                        PluginStateType,
                        SetValueCallback<SliderAttachment>>
        attachment;
};

template <typename PluginStateType>
class ComboBoxAttachment : private juce::ComboBox::Listener
{
public:
    ComboBoxAttachment (ChoiceParameter& param,
                        PluginStateType& pluginState,
                        juce::ComboBox& combo,
                        juce::UndoManager* undoManager = nullptr);

    ~ComboBoxAttachment() override;

    void setValue (int newValue);

private:
    void comboBoxChanged (juce::ComboBox*) override;

    juce::ComboBox& comboBox;
    ParameterAttachment<ChoiceParameter,
                        PluginStateType,
                        SetValueCallback<ComboBoxAttachment>>
        attachment;
};

template <typename PluginStateType>
class ButtonAttachment : private juce::Button::Listener
{
public:
    ButtonAttachment (BoolParameter& param,
                      PluginStateType& pluginState,
                      juce::Button& paramButton,
                      juce::UndoManager* undoManager = nullptr);

    ~ButtonAttachment() override;

    void setValue (bool newValue);

private:
    void buttonClicked (juce::Button*) override;

    juce::Button& button;
    ParameterAttachment<BoolParameter,
                        PluginStateType,
                        SetValueCallback<ButtonAttachment>>
        attachment;
};
} // namespace chowdsp

#include "chowdsp_ParameterAttachments.cpp"
