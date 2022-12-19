namespace chowdsp
{
ButtonAttachment::ButtonAttachment (BoolParameter& param,
                                    PluginState& pluginState,
                                    juce::Button& paramButton)
    : ButtonAttachment (param, pluginState.getParameterListeners(), paramButton, pluginState.undoManager)
{
}

ButtonAttachment::ButtonAttachment (BoolParameter& param,
                                    ParameterListeners& listeners,
                                    juce::Button& paramButton,
                                    juce::UndoManager* undoManager)
    : button (paramButton),
      attachment (param, listeners, ParameterAttachmentHelpers::SetValueCallback { *this }),
      um (undoManager)
{
    button.setButtonText (param.name);
    setValue (param.get());
    button.addListener (this);
}

ButtonAttachment::~ButtonAttachment()
{
    button.removeListener (this);
}

void ButtonAttachment::setValue (bool newValue)
{
    juce::ScopedValueSetter svs { skipClickCallback, true };
    button.setToggleState (newValue, juce::sendNotificationSync);
}

void ButtonAttachment::buttonClicked (juce::Button*)
{
    if (skipClickCallback)
        return;

    const auto newValue = button.getToggleState();
    if (um != nullptr)
    {
        um->beginNewTransaction();
        um->perform (
            new ParameterAttachmentHelpers::ParameterChangeAction<BoolParameter> (
                attachment.param,
                newValue));
    }

    attachment.setValueAsCompleteGesture (newValue);
}
} // namespace chowdsp
