namespace chowdsp
{
template <typename Param, typename State, typename Callback>
ParameterAttachment<Param, State, Callback>::ParameterAttachment (const ParameterPath& parameterPath,
                                                                  State& pluginState,
                                                                  Callback&& callback)
    : ParameterAttachment (pluginState.template getParameter<Param> (parameterPath), pluginState, std::move (callback))
{
}

template <typename Param, typename State, typename Callback>
ParameterAttachment<Param, State, Callback>::ParameterAttachment (Param& parameter,
                                                                  State& pluginState,
                                                                  Callback&& callback)
    : param (parameter)
{
    valueChangedCallback = pluginState.addParameterListener (param,
                                                             true,
                                                             [this, c = std::move (callback)]() mutable
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
} // namespace chowdsp
