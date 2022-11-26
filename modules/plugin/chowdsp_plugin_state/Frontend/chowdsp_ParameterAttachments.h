#pragma once

namespace chowdsp
{
template <typename ParamType, typename PluginStateType, typename Callback>
class ParameterAttachment
{
public:
    ParameterAttachment (ParamType& parameter, PluginStateType& pluginState, Callback&& callback)
        : param (parameter)
    {
        valueChangedCallback = pluginState.addParameterListener (param, true, [this, c = std::forward<Callback> (callback)]
                                                                 { c (ParameterTypeHelpers::getValue (param)); });
    }

    /** Calls the parameterChangedCallback function that was registered in
        the constructor, making the UI reflect the current parameter state.
        This function should be called after doing any necessary setup on
        the UI control that is being managed (e.g. adding ComboBox entries,
        making buttons toggle-able).
    */
    //    void sendInitialUpdate()
    //    {
    //
    //    }

    /** Triggers a full gesture message on the managed parameter.
        Call this in the listener callback of the UI control in response
        to a one-off change in the UI like a button-press.
    */
    void setValueAsCompleteGesture (float newDenormalisedValue)
    {
        callIfParameterValueChanged (newDenormalisedValue,
                                     [this] (float f)
                                     {
                                         beginGesture();
                                         param.setValueNotifyingHost (f);
                                         endGesture();
                                     });
    }

    /** Begins a gesture on the managed parameter.
        Call this when the UI is about to begin a continuous interaction,
        like when the mouse button is pressed on a slider.
    */
    void beginGesture()
    {
        param.beginChangeGesture();
    }

    /** Updates the parameter value during a gesture.
        Call this during a continuous interaction, like a slider value
        changed callback.
    */
    void setValueAsPartOfGesture (float newDenormalisedValue)
    {
        callIfParameterValueChanged (newDenormalisedValue, [this] (float f)
                                     { param.setValueNotifyingHost (f); });
    }

    /** Ends a gesture on the managed parameter.
        Call this when the UI has finished a continuous interaction,
        like when the mouse button is released on a slider.
    */
    void endGesture()
    {
        param.endChangeGesture();
    }

private:
    template <typename Func>
    void callIfParameterValueChanged (float newDenormalisedValue, Func&& func)
    {
        if (ParameterTypeHelpers::getValue (param) != newDenormalisedValue)
            func (newDenormalisedValue);
    }

    ParamType& param;
    chowdsp::ScopedCallback valueChangedCallback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterAttachment)
};
} // namespace chowdsp
