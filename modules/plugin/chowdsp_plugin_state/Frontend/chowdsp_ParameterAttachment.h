#pragma once

namespace chowdsp
{
/** An attachment onject that can be used implement UI controllers for attachments. */
template <typename ParamType, typename PluginStateType, typename Callback = std::function<void (ParameterTypeHelpers::ParameterElementType<ParamType>)>>
class ParameterAttachment
{
public:
    /** Creates an attachment for a given parameter path and plugin state. */
    ParameterAttachment (const ParameterPath& parameterPath,
                         PluginStateType& pluginState,
                         Callback&& callback);

    /** Creates an attachment for a given parameter and plugin state. */
    ParameterAttachment (ParamType& parameter,
                         PluginStateType& pluginState,
                         Callback&& callback);

    using ParamElementType = ParameterTypeHelpers::ParameterElementType<ParamType>;

    /**
     * Begins a gesture on the managed parameter.
     * Call this when the UI is about to begin a continuous interaction,
     * like when the mouse button is pressed on a slider.
     */
    void beginGesture();

    /**
     * Ends a gesture on the managed parameter.
     * Call this when the UI has finished a continuous interaction,
     * like when the mouse button is released on a slider.
     */
    void endGesture();

    /**
     * Triggers a full gesture message on the managed parameter.
     * Call this in the listener callback of the UI control in response
     * to a one-off change in the UI like a button-press.
     */
    void setValueAsCompleteGesture (ParamElementType newValue);

    /**
     * Updates the parameter value during a gesture.
     * Call this during a continuous interaction, like a slider value
     * changed callback.
     */
    void setValueAsPartOfGesture (ParamElementType newValue);

    ParamType& param;

private:
    template <typename Func>
    void callIfParameterValueChanged (ParamElementType newValue, Func&& func);

    ScopedCallback valueChangedCallback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterAttachment)
};
} // namespace chowdsp

#include "chowdsp_ParameterAttachment.cpp"
