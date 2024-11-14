namespace chowdsp
{
SliderChoiceAttachment::SliderChoiceAttachment (ChoiceParameter& param,
                                                PluginState& pluginState,
                                                juce::Slider& paramSlider)
    : SliderChoiceAttachment (param, pluginState.getParameterListeners(), paramSlider, pluginState.undoManager)
{
    attachment.pluginState = &pluginState;
}

SliderChoiceAttachment::SliderChoiceAttachment (ChoiceParameter& param,
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
    slider->setDoubleClickReturnValue (true, param.getDefaultIndex());

    slider->setRange (0.0, static_cast<double> (param.choices.size() - 1), 1.0);

    setValue (param.getIndex());
    slider->valueChanged();
    slider->addListener (this);
}

SliderChoiceAttachment::~SliderChoiceAttachment()
{
    if (slider != nullptr)
        slider->removeListener (this);
}

void SliderChoiceAttachment::setValue (int newValue)
{
    if (slider != nullptr)
    {
        juce::ScopedValueSetter svs { skipSliderChangedCallback, true };
        slider->setValue (static_cast<double> (newValue), juce::sendNotificationSync);
    }
}

void SliderChoiceAttachment::sliderValueChanged (juce::Slider*)
{
    if (skipSliderChangedCallback)
        return;

    attachment.setValueAsPartOfGesture (static_cast<int> (slider->getValue()));
}

void SliderChoiceAttachment::sliderDragStarted (juce::Slider*)
{
    valueAtStartOfGesture = attachment.param->getIndex();
    attachment.beginGesture();
}

void SliderChoiceAttachment::sliderDragEnded (juce::Slider*)
{
    const auto valueAtEndOfGesture = attachment.param->getIndex();
    if (um != nullptr && valueAtEndOfGesture != valueAtStartOfGesture)
    {
        um->beginNewTransaction();
        um->perform (
            new ParameterAttachmentHelpers::ParameterChangeAction<ChoiceParameter> (
                *attachment.param,
                valueAtStartOfGesture,
                valueAtEndOfGesture,
                attachment.pluginState == nullptr ? nullptr : attachment.pluginState->processor));
    }

    attachment.endGesture();
}
} // namespace chowdsp
