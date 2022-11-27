namespace chowdsp
{
template <typename Param, typename State, typename Callback>
ParameterAttachment<Param, State, Callback>::ParameterAttachment (Param& parameter,
                                                                  State& pluginState,
                                                                  Callback&& callback)
    : param (parameter)
{
    valueChangedCallback = pluginState.addParameterListener (param,
                                                             true,
                                                             [this, c = std::forward<Callback> (callback)]() mutable
                                                             {
                                                                 c (ParameterTypeHelpers::getValue (param));
                                                             });
}

template <typename Param, typename State, typename Callback>
void ParameterAttachment<Param, State, Callback>::beginGesture()
{
    param.beginChangeGesture();
}

template <typename Param, typename State, typename Callback>
void ParameterAttachment<Param, State, Callback>::endGesture()
{
    param.endChangeGesture();
}

template <typename Param, typename State, typename Callback>
void ParameterAttachment<Param, State, Callback>::setValueAsCompleteGesture (ParamElementType newValue)
{
    callIfParameterValueChanged (newValue,
                                 [this] (ParamElementType val)
                                 {
                                     beginGesture();
                                     ParameterTypeHelpers::setValue (val, param);
                                     endGesture();
                                 });
}

template <typename Param, typename State, typename Callback>
void ParameterAttachment<Param, State, Callback>::setValueAsPartOfGesture (ParamElementType newValue)
{
    callIfParameterValueChanged (newValue,
                                 [this] (ParamElementType val)
                                 {
                                     ParameterTypeHelpers::setValue (val, param);
                                 });
}

template <typename Param, typename State, typename Callback>
template <typename Func>
void ParameterAttachment<Param, State, Callback>::callIfParameterValueChanged (ParamElementType newValue,
                                                                               Func&& func)
{
    if (ParameterTypeHelpers::getValue (param) != newValue)
        func (newValue);
}

//======================================================================
template <typename State>
SliderAttachment<State>::SliderAttachment (FloatParameter& param,
                                           State& pluginState,
                                           juce::Slider& paramSlider,
                                           juce::UndoManager* undoManager)
    : slider (paramSlider),
      attachment (param, pluginState, SetValueCallback { *this }),
      um (undoManager != nullptr ? undoManager : pluginState.undoManager)
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
    if (slider.getValue() == (double) newValue)
        return;

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
        um->perform (new ParameterChangeAction (attachment,
                                                valueAtStartOfGesture,
                                                attachment.param.get()));
    }

    attachment.endGesture();
}

//======================================================================
template <typename State>
ComboBoxAttachment<State>::ComboBoxAttachment (ChoiceParameter& param,
                                               State& pluginState,
                                               juce::ComboBox& combo,
                                               juce::UndoManager* undoManager)
    : comboBox (combo),
      attachment (param, pluginState, SetValueCallback { *this }),
      um (undoManager != nullptr ? undoManager : pluginState.undoManager)
{
    comboBox.addItemList (param.choices, 1);
    setValue (param.getIndex());
    comboBox.addListener (this);
}

template <typename State>
ComboBoxAttachment<State>::~ComboBoxAttachment()
{
    comboBox.removeListener (this);
}

template <typename State>
void ComboBoxAttachment<State>::setValue (int newValue)
{
    if (comboBox.getSelectedItemIndex() == newValue)
        return;

    juce::ScopedValueSetter svs { skipBoxChangedCallback, true };
    comboBox.setSelectedItemIndex (newValue, juce::sendNotificationSync);
}

template <typename State>
void ComboBoxAttachment<State>::comboBoxChanged (juce::ComboBox*)
{
    if (skipBoxChangedCallback)
        return;

    const auto newValue = comboBox.getSelectedItemIndex();

    if (um != nullptr)
    {
        um->beginNewTransaction();
        um->perform (new ParameterChangeAction (attachment,
                                                attachment.param.getIndex(),
                                                newValue));
    }

    attachment.setValueAsCompleteGesture (newValue);
}

//======================================================================
template <typename State>
ButtonAttachment<State>::ButtonAttachment (BoolParameter& param,
                                           State& pluginState,
                                           juce::Button& paramButton,
                                           juce::UndoManager* undoManager)
    : button (paramButton),
      attachment (param, pluginState, SetValueCallback { *this }),
      um (undoManager != nullptr ? undoManager : pluginState.undoManager)
{
    button.setButtonText (param.name);
    setValue (param.get());
    button.addListener (this);
}

template <typename State>
ButtonAttachment<State>::~ButtonAttachment()
{
    button.removeListener (this);
}

template <typename State>
void ButtonAttachment<State>::setValue (bool newValue)
{
    if (button.getToggleState() == newValue)
        return;

    juce::ScopedValueSetter svs { skipClickCallback, true };
    button.setToggleState (newValue, juce::sendNotificationSync);
}

template <typename State>
void ButtonAttachment<State>::buttonClicked (juce::Button*)
{
    if (skipClickCallback)
        return;

    const auto newValue = button.getToggleState();
    if (um != nullptr)
    {
        um->beginNewTransaction();
        um->perform (new ParameterChangeAction (attachment,
                                                attachment.param.get(),
                                                newValue));
    }

    attachment.setValueAsCompleteGesture (newValue);
}
} // namespace chowdsp
