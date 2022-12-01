namespace chowdsp
{
template <typename State>
ButtonAttachment<State>::ButtonAttachment (const ParameterPath& paramPath,
                                           State& pluginState,
                                           juce::Button& paramButton)
    : ButtonAttachment (pluginState.template getParameter<BoolParameter> (paramPath),
                        pluginState,
                        paramButton)
{
}

template <typename State>
ButtonAttachment<State>::ButtonAttachment (BoolParameter& param,
                                           State& pluginState,
                                           juce::Button& paramButton)
    : ButtonAttachment (param, pluginState, paramButton, pluginState.undoManager)
{
}

template <typename State>
ButtonAttachment<State>::ButtonAttachment (BoolParameter& param,
                                           State& pluginState,
                                           juce::Button& paramButton,
                                           juce::UndoManager* undoManager)
    : button (paramButton),
      attachment (param, pluginState, ParameterAttachmentHelpers::SetValueCallback { *this }),
      um (undoManager)
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
        um->perform (new ParameterAttachmentHelpers::ParameterChangeAction (attachment,
                                                                            attachment.param.get(),
                                                                            newValue));
    }

    attachment.setValueAsCompleteGesture (newValue);
}
} // namespace chowdsp
