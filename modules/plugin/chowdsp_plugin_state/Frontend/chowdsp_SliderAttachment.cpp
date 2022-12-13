namespace chowdsp
{
template <typename State>
SliderAttachment<State>::SliderAttachment (const ParameterPath& paramPath,
                                           State& pluginState,
                                           juce::Slider& paramSlider)
    : SliderAttachment (pluginState.template getParameter<FloatParameter> (paramPath),
                        pluginState,
                        paramSlider)
{
}

template <typename State>
SliderAttachment<State>::SliderAttachment (FloatParameter& param,
                                           State& pluginState,
                                           juce::Slider& paramSlider)
    : SliderAttachment (param, pluginState, paramSlider, pluginState.undoManager)
{
}

template <typename State>
SliderAttachment<State>::SliderAttachment (FloatParameter& param,
                                           State& pluginState,
                                           juce::Slider& paramSlider,
                                           juce::UndoManager* undoManager)
    : slider (paramSlider),
      attachment (param, pluginState, ParameterAttachmentHelpers::SetValueCallback { *this }),
      um (undoManager)
{
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

template <typename State>
SliderAttachment<State>::~SliderAttachment()
{
    slider.removeListener (this);
}

template <typename State>
void SliderAttachment<State>::setValue (float newValue)
{
    juce::ScopedValueSetter svs { skipSliderChangedCallback, true };
    slider.setValue (newValue, juce::sendNotificationSync);
}

template <typename State>
void SliderAttachment<State>::sliderValueChanged (juce::Slider*)
{
    if (skipSliderChangedCallback)
        return;

    attachment.setValueAsPartOfGesture ((float) slider.getValue());
}

template <typename State>
void SliderAttachment<State>::sliderDragStarted (juce::Slider*)
{
    valueAtStartOfGesture = attachment.param.get();
    attachment.beginGesture();
}

template <typename State>
void SliderAttachment<State>::sliderDragEnded (juce::Slider*)
{
    if (um != nullptr)
    {
        um->beginNewTransaction();
        um->perform (new ParameterAttachmentHelpers::ParameterChangeAction (attachment,
                                                                            valueAtStartOfGesture,
                                                                            attachment.param.get()));
    }

    attachment.endGesture();
}
} // namespace chowdsp
