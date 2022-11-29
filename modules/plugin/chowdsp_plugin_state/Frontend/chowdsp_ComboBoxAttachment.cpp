namespace chowdsp
{
template <typename State>
ComboBoxAttachment<State>::ComboBoxAttachment (const ParameterPath& paramPath,
                                               State& pluginState,
                                               juce::ComboBox& combo)
    : ComboBoxAttachment (pluginState.template getParameter<ChoiceParameter> (paramPath),
                          pluginState,
                          combo)
{
}

template <typename State>
ComboBoxAttachment<State>::ComboBoxAttachment (ChoiceParameter& param,
                                               State& pluginState,
                                               juce::ComboBox& combo)
    : ComboBoxAttachment (param, pluginState, combo, pluginState.undoManager)
{
}

template <typename State>
ComboBoxAttachment<State>::ComboBoxAttachment (ChoiceParameter& param,
                                               State& pluginState,
                                               juce::ComboBox& combo,
                                               juce::UndoManager* undoManager)
    : comboBox (combo),
      attachment (param, pluginState, ParameterAttachmentHelpers::SetValueCallback { *this }),
      um (undoManager)
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
        um->perform (new ParameterAttachmentHelpers::ParameterChangeAction (attachment,
                                                                            attachment.param.getIndex(),
                                                                            newValue));
    }

    attachment.setValueAsCompleteGesture (newValue);
}
} // namespace chowdsp
