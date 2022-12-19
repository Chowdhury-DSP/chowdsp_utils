namespace chowdsp
{
ComboBoxAttachment::ComboBoxAttachment (ChoiceParameter& param,
                                        PluginState& pluginState,
                                        juce::ComboBox& combo)
    : ComboBoxAttachment (param, pluginState.getParameterListeners(), combo, pluginState.undoManager)
{
}

ComboBoxAttachment::ComboBoxAttachment (ChoiceParameter& param,
                                        ParameterListeners& listeners,
                                        juce::ComboBox& combo,
                                        juce::UndoManager* undoManager)
    : comboBox (combo),
      attachment (param, listeners, ParameterAttachmentHelpers::SetValueCallback { *this }),
      um (undoManager)
{
    comboBox.addItemList (param.choices, 1);
    setValue (param.getIndex());
    comboBox.addListener (this);
}

ComboBoxAttachment::~ComboBoxAttachment()
{
    comboBox.removeListener (this);
}

void ComboBoxAttachment::setValue (int newValue)
{
    juce::ScopedValueSetter svs { skipBoxChangedCallback, true };
    comboBox.setSelectedItemIndex (newValue, juce::sendNotificationSync);
}

void ComboBoxAttachment::comboBoxChanged (juce::ComboBox*)
{
    if (skipBoxChangedCallback)
        return;

    const auto newValue = comboBox.getSelectedItemIndex();

    if (um != nullptr)
    {
        um->beginNewTransaction();
        um->perform (
            new ParameterAttachmentHelpers::ParameterChangeAction<ChoiceParameter> (
                attachment.param,
                attachment.param.getIndex(),
                newValue));
    }

    attachment.setValueAsCompleteGesture (newValue);
}
} // namespace chowdsp
