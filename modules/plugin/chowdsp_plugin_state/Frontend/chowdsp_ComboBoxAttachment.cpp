namespace chowdsp
{
ComboBoxAttachment::ComboBoxAttachment (ChoiceParameter& param,
                                        PluginState& pluginState,
                                        juce::ComboBox& combo)
    : ComboBoxAttachment (param, pluginState.getParameterListeners(), combo, pluginState.undoManager)
{
    attachment.pluginState = &pluginState;
}

ComboBoxAttachment::ComboBoxAttachment (ChoiceParameter& param,
                                        ParameterListeners& listeners,
                                        juce::ComboBox& combo,
                                        juce::UndoManager* undoManager)
    : comboBox (&combo),
      attachment (param, listeners, ParameterAttachmentHelpers::SetValueCallback { *this }),
      um (undoManager)
{
    comboBox->addItemList (param.choices, 1);
    setValue (param.getIndex());
    comboBox->addListener (this);
}

ComboBoxAttachment::~ComboBoxAttachment()
{
    if (comboBox != nullptr)
        comboBox->removeListener (this);
}

void ComboBoxAttachment::setValue (int newValue)
{
    if (comboBox != nullptr)
    {
        juce::ScopedValueSetter svs { skipBoxChangedCallback, true };
        comboBox->setSelectedItemIndex (newValue, juce::sendNotificationSync);
    }
}

void ComboBoxAttachment::comboBoxChanged (juce::ComboBox*)
{
    if (skipBoxChangedCallback)
        return;

    const auto newValue = comboBox->getSelectedItemIndex();
    attachment.setValueAsCompleteGesture (newValue, um);
}
} // namespace chowdsp
