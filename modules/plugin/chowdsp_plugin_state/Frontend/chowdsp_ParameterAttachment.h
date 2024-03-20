#pragma once

namespace chowdsp
{
/** An attachment onject that can be used implement UI controllers for attachments. */
template <typename ParamType, typename Callback = std::function<void (ParameterTypeHelpers::ParameterElementType<ParamType>)>>
class ParameterAttachment
{
public:
    /** Creates an attachment for a given parameter and plugin state. */
    ParameterAttachment (ParamType& parameter,
                         PluginState& pluginState,
                         Callback&& callback);

    /** Creates an attachment for a given parameter and parameter listeners. */
    ParameterAttachment (ParamType& parameter,
                         ParameterListeners& listeners,
                         Callback&& callback);

    ParameterAttachment() = default;
    ParameterAttachment (const ParameterAttachment&) = default;
    ParameterAttachment& operator= (const ParameterAttachment&) = default;
    ParameterAttachment (ParameterAttachment&&) noexcept = default;
    ParameterAttachment& operator= (ParameterAttachment&&) noexcept = default;

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
    void setValueAsCompleteGesture (ParamElementType newValue, juce::UndoManager* um = nullptr);

    /**
     * Updates the parameter value during a gesture.
     * Call this during a continuous interaction, like a slider value
     * changed callback.
     */
    void setValueAsPartOfGesture (ParamElementType newValue);

    /** Manually triggers an update as though the parameter has changed. */
    void manuallyTriggerUpdate() const;

    ParamType* param = nullptr;
    PluginState* pluginState = nullptr;

private:
    template <typename Func>
    void callIfParameterValueChanged (ParamElementType newValue, Func&& func);

    Callback updateCallback {};
    ScopedCallback valueChangedCallback;

    JUCE_LEAK_DETECTOR (ParameterAttachment)
};
} // namespace chowdsp

#include "chowdsp_ParameterAttachment.cpp"
