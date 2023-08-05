#include "chowdsp_ParameterAttachmentHelpers.h"

namespace chowdsp
{
template <typename Param, typename Callback>
ParameterAttachment<Param, Callback>::ParameterAttachment (Param& parameter,
                                                           PluginState& pluginState,
                                                           Callback&& callback)
    : ParameterAttachment (parameter, pluginState.getParameterListeners(), std::forward<Callback> (callback))
{
}

template <typename Param, typename Callback>
ParameterAttachment<Param, Callback>::ParameterAttachment (Param& parameter,
                                                           ParameterListeners& listeners,
                                                           Callback&& callback)
    : param (&parameter)
{
    valueChangedCallback = listeners.addParameterListener (*param,
                                                           ParameterListenerThread::MessageThread,
                                                           [this, c = std::move (callback)]() mutable
                                                           {
                                                               if (param != nullptr)
                                                                   c (ParameterTypeHelpers::getValue (*param));
                                                           });
}

template <typename Param, typename Callback>
void ParameterAttachment<Param, Callback>::beginGesture()
{
    if (param != nullptr)
        param->beginChangeGesture();
}

template <typename Param, typename Callback>
void ParameterAttachment<Param, Callback>::endGesture()
{
    if (param != nullptr)
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
                                                 val));
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
