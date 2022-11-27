#pragma once

namespace chowdsp
{
// @TODO: deal with UndoManager
template <typename ParamType, typename PluginStateType, typename Callback = std::function<void (ParameterTypeHelpers::ParameterElementType<ParamType>)>>
class ParameterAttachment
{
public:
    ParameterAttachment (ParamType& parameter, PluginStateType& pluginState, Callback&& callback)
        : param (parameter)
    {
        valueChangedCallback = pluginState.addParameterListener (param,
                                                                 true,
                                                                 [this, c = std::forward<Callback> (callback)]() mutable
                                                                 {
                                                                     c (ParameterTypeHelpers::getValue (param));
                                                                 });
    }

    using ParamElementType = ParameterTypeHelpers::ParameterElementType<ParamType>;

    /** Triggers a full gesture message on the managed parameter.
        Call this in the listener callback of the UI control in response
        to a one-off change in the UI like a button-press.
    */
    void setValueAsCompleteGesture (ParamElementType newValue)
    {
        callIfParameterValueChanged (newValue,
                                     [this] (ParamElementType val)
                                     {
                                         beginGesture();
                                         ParameterTypeHelpers::setValue (val, param);
                                         endGesture();
                                     });
    }

    /** Begins a gesture on the managed parameter.
        Call this when the UI is about to begin a continuous interaction,
        like when the mouse button is pressed on a slider.
    */
    void beginGesture()
    {
        param.beginChangeGesture();
    }

    /** Updates the parameter value during a gesture.
        Call this during a continuous interaction, like a slider value
        changed callback.
    */
    void setValueAsPartOfGesture (ParamElementType newValue)
    {
        callIfParameterValueChanged (newValue,
                                     [this] (ParamElementType val)
                                     {
                                         ParameterTypeHelpers::setValue (val, param);
                                     });
    }

    /** Ends a gesture on the managed parameter.
        Call this when the UI has finished a continuous interaction,
        like when the mouse button is released on a slider.
    */
    void endGesture()
    {
        param.endChangeGesture();
    }

private:
    template <typename Func>
    void callIfParameterValueChanged (ParamElementType newValue, Func&& func)
    {
        if (ParameterTypeHelpers::getValue (param) != newValue)
            func (newValue);
    }

    ParamType& param;
    chowdsp::ScopedCallback valueChangedCallback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterAttachment)
};

template <typename PluginStateType>
class SliderAttachment : private juce::Slider::Listener
{
public:
    SliderAttachment (FloatParameter& param,
                      PluginStateType& pluginState,
                      juce::Slider& paramSlider,
                      juce::UndoManager* undoManager = nullptr)
        : slider (paramSlider),
          attachment (param, pluginState, SetValueCallback { *this })
    {
        juce::ignoreUnused (undoManager);

        slider.valueFromTextFunction = [&p = static_cast<juce::RangedAudioParameter&> (param)] (const juce::String& text)
        {
            return (double) p.convertFrom0to1 (p.getValueForText (text));
        };
        slider.textFromValueFunction = [&p = static_cast<juce::RangedAudioParameter&> (param)] (double value)
        {
            return p.getText (p.convertTo0to1 ((float) value), 0);
        };
        slider.setDoubleClickReturnValue (true, param.convertFrom0to1 (param.getDefaultValue()));

        auto range = param.getNormalisableRange();

        auto convertFrom0To1Function = [range] (double currentRangeStart,
                                                double currentRangeEnd,
                                                double normalisedValue) mutable
        {
            range.start = (float) currentRangeStart;
            range.end = (float) currentRangeEnd;
            return (double) range.convertFrom0to1 ((float) normalisedValue);
        };

        auto convertTo0To1Function = [range] (double currentRangeStart,
                                              double currentRangeEnd,
                                              double mappedValue) mutable
        {
            range.start = (float) currentRangeStart;
            range.end = (float) currentRangeEnd;
            return (double) range.convertTo0to1 ((float) mappedValue);
        };

        auto snapToLegalValueFunction = [range] (double currentRangeStart,
                                                 double currentRangeEnd,
                                                 double mappedValue) mutable
        {
            range.start = (float) currentRangeStart;
            range.end = (float) currentRangeEnd;
            return (double) range.snapToLegalValue ((float) mappedValue);
        };

        juce::NormalisableRange<double> newRange { (double) range.start,
                                                   (double) range.end,
                                                   std::move (convertFrom0To1Function),
                                                   std::move (convertTo0To1Function),
                                                   std::move (snapToLegalValueFunction) };
        newRange.interval = range.interval;
        newRange.skew = range.skew;
        newRange.symmetricSkew = range.symmetricSkew;

        slider.setNormalisableRange (newRange);

        setValue (param.get());
        slider.valueChanged();
        slider.addListener (this);
    }

    ~SliderAttachment() override
    {
        slider.removeListener (this);
    }

    void setValue (float newValue)
    {
        if (slider.getValue() != (double) newValue)
            slider.setValue (newValue, juce::sendNotificationSync);
    }

private:
    void sliderValueChanged (juce::Slider*) override
    {
        attachment.setValueAsPartOfGesture ((float) slider.getValue());
    }

    void sliderDragStarted (juce::Slider*) override { attachment.beginGesture(); }
    void sliderDragEnded (juce::Slider*) override { attachment.endGesture(); }

    juce::Slider& slider;

    struct SetValueCallback
    {
        explicit SetValueCallback (SliderAttachment& sa) : attach (sa) {}
        void operator() (float val) { attach.setValue (val); }
        SliderAttachment& attach;
    };

    ParameterAttachment<FloatParameter, PluginStateType, SetValueCallback> attachment;
};
} // namespace chowdsp
