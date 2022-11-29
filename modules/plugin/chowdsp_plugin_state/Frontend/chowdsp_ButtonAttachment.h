#pragma once

namespace chowdsp
{
/**
 * An object of this class maintains a connection between a juce::Button and a plug-in parameter.
 *
 * During the lifetime of this object it keeps the two things in sync, making it easy to connect a button
 * to a parameter. When this object is deleted, the connection is broken. Make sure that your parameter
 * and Button are not deleted before this object!
 */
template <typename PluginStateType>
class ButtonAttachment : private juce::Button::Listener
{
public:
    /** Creates an attachment for a given parameter path */
    ButtonAttachment (const ParameterPath& paramPath,
                      PluginStateType& pluginState,
                      juce::Button& paramButton);

    /** Creates an attachment for a given parameter, using the undo manager from the plugin state  */
    ButtonAttachment (BoolParameter& param,
                      PluginStateType& pluginState,
                      juce::Button& paramButton);

    /** Creates an attachment for a given parameter, using a custom UndoManager */
    ButtonAttachment (BoolParameter& param,
                      PluginStateType& pluginState,
                      juce::Button& paramButton,
                      juce::UndoManager* undoManager);

    ~ButtonAttachment() override;

    /** Sets the initial value of the button */
    void setValue (bool newValue);

private:
    void buttonClicked (juce::Button*) override;

    juce::Button& button;
    ParameterAttachment<BoolParameter,
                        PluginStateType,
                        ParameterAttachmentHelpers::SetValueCallback<ButtonAttachment>>
        attachment;
    juce::UndoManager* um = nullptr;

    bool skipClickCallback = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ButtonAttachment)
};
} // namespace chowdsp

#include "chowdsp_ButtonAttachment.cpp"
