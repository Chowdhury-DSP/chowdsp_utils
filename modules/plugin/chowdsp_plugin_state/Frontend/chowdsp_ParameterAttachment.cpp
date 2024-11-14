#include "chowdsp_ParameterAttachmentHelpers.h"

namespace chowdsp
{
template <typename Param, typename Callback>
ParameterAttachment<Param, Callback>::ParameterAttachment (Param& parameter,
                                                           PluginState& plugState,
                                                           Callback&& callback)
    : ParameterAttachment (parameter, plugState.getParameterListeners(), std::forward<Callback> (callback))
{
    pluginState = &plugState;
}

template <typename Param, typename Callback>
ParameterAttachment<Param, Callback>::ParameterAttachment (Param& parameter,
                                                           ParameterListeners& listeners,
                                                           Callback&& callback)
    : param (&parameter),
      updateCallback (std::move (callback))
{
    valueChangedCallback = listeners.addParameterListener (*param,
                                                           ParameterListenerThread::MessageThread,
                                                           [this]() mutable
                                                           {
                                                               if (param != nullptr)
                                                                   updateCallback (ParameterTypeHelpers::getValue (*param));
                                                           });
}

template <typename Param, typename Callback>
void ParameterAttachment<Param, Callback>::beginGesture()
{
    if (param != nullptr && pluginState != nullptr && pluginState->processor != nullptr)
        param->beginChangeGesture();
}

template <typename Param, typename Callback>
void ParameterAttachment<Param, Callback>::endGesture()
{
    if (param != nullptr && pluginState != nullptr && pluginState->processor != nullptr)
        param->endChangeGesture();
}

template <typename Param, typename Callback>
void ParameterAttachment<Param, Callback>::setValueAsCompleteGesture (ParamElementType newValue, juce::UndoManager* um)
{
    callIfParameterValueChanged (newValue,
                                 [this, &um] (ParamElementType val)
                                 {
                                     if (um != nullptr)
                                     {
                                         um->beginNewTransaction();
                                         um->perform (
                                             new ParameterAttachmentHelpers::ParameterChangeAction<Param> (
                                                 *param,
                                                 ParameterTypeHelpers::getValue (*param),
                                                 val,
                                                 pluginState == nullptr ? nullptr : pluginState->processor));
                                     }

                                     beginGesture();
                                     ParameterTypeHelpers::setValue (val, *param);
                                     endGesture();
                                 });
}

template <typename Param, typename Callback>
void ParameterAttachment<Param, Callback>::setValueAsPartOfGesture (ParamElementType newValue)
{
    callIfParameterValueChanged (newValue,
                                 [this] (ParamElementType val)
                                 {
                                     ParameterTypeHelpers::setValue (val, *param);
                                 });
}

template <typename Param, typename Callback>
void ParameterAttachment<Param, Callback>::manuallyTriggerUpdate() const
{
    if (param != nullptr)
        updateCallback (ParameterTypeHelpers::getValue (*param));
}

template <typename Param, typename Callback>
template <typename Func>
void ParameterAttachment<Param, Callback>::callIfParameterValueChanged (ParamElementType newValue,
                                                                        Func&& func)
{
    if (param == nullptr)
        return;

    if constexpr (std::is_floating_point_v<ParamElementType>)
    {
        if (juce::approximatelyEqual (ParameterTypeHelpers::getValue (*param), newValue))
            return;
    }
    else
    {
        if (ParameterTypeHelpers::getValue (*param) == newValue)
            return;
    }

    func (newValue);
}
} // namespace chowdsp
