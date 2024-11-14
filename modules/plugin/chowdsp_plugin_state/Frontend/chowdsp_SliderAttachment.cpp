namespace chowdsp
{
SliderAttachment::SliderAttachment (FloatParameter& param,
                                    PluginState& pluginState,
                                    juce::Slider& paramSlider)
    : SliderAttachment (param, pluginState.getParameterListeners(), paramSlider, pluginState.undoManager)
{
    attachment.pluginState = &pluginState;
}

SliderAttachment::SliderAttachment (FloatParameter& param,
                                    ParameterListeners& listeners,
                                    juce::Slider& paramSlider,
                                    juce::UndoManager* undoManager)
    : slider (&paramSlider),
      attachment (param, listeners, ParameterAttachmentHelpers::SetValueCallback { *this }),
      um (undoManager)
{
    slider->valueFromTextFunction = [&p = static_cast<juce::RangedAudioParameter&> (param)] (const juce::String& text)
    {
        return (double) p.convertFrom0to1 (p.getValueForText (text));
    };
    slider->textFromValueFunction = [&p = static_cast<juce::RangedAudioParameter&> (param)] (double value)
    {
        return p.getText (p.convertTo0to1 ((float) value), 0);
    };
    slider->setDoubleClickReturnValue (true, param.convertFrom0to1 (param.getDefaultValue()));

    auto range = param.getNormalisableRange(); // NOSONAR

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

    auto newRange = juce::NormalisableRange<double> { // NOSONAR
                                                      (double) range.start,
                                                      (double) range.end,
                                                      std::move (convertFrom0To1Function),
                                                      std::move (convertTo0To1Function),
                                                      std::move (snapToLegalValueFunction)
    };
    newRange.interval = range.interval;
    newRange.skew = range.skew;
    newRange.symmetricSkew = range.symmetricSkew;

    slider->setNormalisableRange (newRange);

    setValue (param.get());
    slider->valueChanged();
    slider->addListener (this);
}

SliderAttachment::~SliderAttachment()
{
    if (slider != nullptr)
        slider->removeListener (this);
}

void SliderAttachment::setValue (float newValue)
{
    if (slider != nullptr)
    {
        juce::ScopedValueSetter svs { skipSliderChangedCallback, true };
        slider->setValue (newValue, juce::sendNotificationSync);
    }
}

void SliderAttachment::sliderValueChanged (juce::Slider*)
{
    if (skipSliderChangedCallback)
        return;

    attachment.setValueAsPartOfGesture ((float) slider->getValue());
}

void SliderAttachment::sliderDragStarted (juce::Slider*)
{
    valueAtStartOfGesture = attachment.param->get();
    attachment.beginGesture();
}

void SliderAttachment::sliderDragEnded (juce::Slider*)
{
    const auto valueAtEndOfGesture = attachment.param->get();
    if (um != nullptr && ! juce::approximatelyEqual (valueAtStartOfGesture, valueAtEndOfGesture))
    {
        um->beginNewTransaction();
        um->perform (
            new ParameterAttachmentHelpers::ParameterChangeAction<FloatParameter> (
                *attachment.param,
                valueAtStartOfGesture,
                valueAtEndOfGesture,
                attachment.pluginState == nullptr ? nullptr : attachment.pluginState->processor));
    }

    attachment.endGesture();
}
} // namespace chowdsp
