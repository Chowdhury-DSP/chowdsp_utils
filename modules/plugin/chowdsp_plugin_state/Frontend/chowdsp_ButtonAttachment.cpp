namespace chowdsp
{
ButtonAttachment::ButtonAttachment (BoolParameter& param,
                                    PluginState& pluginState,
                                    juce::Button& paramButton)
    : ButtonAttachment (param, pluginState.getParameterListeners(), paramButton, pluginState.undoManager)
{
    attachment.pluginState = &pluginState;
}

ButtonAttachment::ButtonAttachment (BoolParameter& param,
                                    ParameterListeners& listeners,
                                    juce::Button& paramButton,
                                    juce::UndoManager* undoManager)
    : button (&paramButton),
      attachment (param, listeners, ParameterAttachmentHelpers::SetValueCallback { *this }),
      um (undoManager)
{
    button->setButtonText (param.name);
    setValue (param.get());
    button->addListener (this);
}

ButtonAttachment::~ButtonAttachment()
{
    if (button != nullptr)
        button->removeListener (this);
}

void ButtonAttachment::setValue (bool newValue)
{
    if (button != nullptr)
    {
        juce::ScopedValueSetter svs { skipClickCallback, true };
        button->setToggleState (newValue, juce::sendNotificationSync);
    }
}

void ButtonAttachment::buttonClicked (juce::Button*)
{
    if (skipClickCallback)
        return;

    const auto newValue = button->getToggleState();
    attachment.setValueAsCompleteGesture (newValue, um);
}
} // namespace chowdsp
