namespace chowdsp
{
template <typename Param, typename Callback>
ParameterAttachment<Param, Callback>::ParameterAttachment (Param& parameter,
                                                           PluginState& pluginState,
                                                           Callback&& callback)
    : ParameterAttachment (parameter, pluginState.getParameterListeners(), std::move<Callback> (callback))
{
}

template <typename Param, typename Callback>
ParameterAttachment<Param, Callback>::ParameterAttachment (Param& parameter,
                                                           ParameterListeners& listeners,
                                                           Callback&& callback)
    : param (parameter)
{
    valueChangedCallback = listeners.addParameterListener (param,
                                                           ParameterListenerThread::MessageThread,
                                                           [this, c = std::move (callback)]() mutable
                                                           {
                                                               c (ParameterTypeHelpers::getValue (param));
                                                           });
}

template <typename Param, typename Callback>
void ParameterAttachment<Param, Callback>::beginGesture()
{
    param.beginChangeGesture();
}

template <typename Param, typename Callback>
void ParameterAttachment<Param, Callback>::endGesture()
{
    param.endChangeGesture();
}

template <typename Param, typename Callback>
void ParameterAttachment<Param, Callback>::setValueAsCompleteGesture (ParamElementType newValue)
{
    callIfParameterValueChanged (newValue,
                                 [this] (ParamElementType val)
                                 {
                                     beginGesture();
                                     ParameterTypeHelpers::setValue (val, param);
                                     endGesture();
                                 });
}

template <typename Param, typename Callback>
void ParameterAttachment<Param, Callback>::setValueAsPartOfGesture (ParamElementType newValue)
{
    callIfParameterValueChanged (newValue,
                                 [this] (ParamElementType val)
                                 {
                                     ParameterTypeHelpers::setValue (val, param);
                                 });
}

template <typename Param, typename Callback>
template <typename Func>
void ParameterAttachment<Param, Callback>::callIfParameterValueChanged (ParamElementType newValue,
                                                                        Func&& func)
{
    if (ParameterTypeHelpers::getValue (param) != newValue)
        func (newValue);
}
} // namespace chowdsp
