#pragma once

namespace chowdsp
{
/**
 * An object of this class maintains a connection between a juce::ComboBox and a plug-in parameter.
 *
 * During the lifetime of this object it keeps the two things in sync, making it easy to connect a box
 * to a parameter. When this object is deleted, the connection is broken. Make sure that your parameter
 * and ComboBox are not deleted before this object!
 */
template <typename PluginStateType>
class ComboBoxAttachment : private juce::ComboBox::Listener
{
public:
    /** Creates an attachment for a given parameter path */
    ComboBoxAttachment (const ParameterPath& paramPath,
                        PluginStateType& pluginState,
                        juce::ComboBox& combo);

    /** Creates an attachment for a given parameter, using the undo manager from the plugin state */
    ComboBoxAttachment (ChoiceParameter& param,
                        PluginStateType& pluginState,
                        juce::ComboBox& combo);

    /** Creates an attachment for a given parameter, using a custon UndoManager */
    ComboBoxAttachment (ChoiceParameter& param,
                        PluginStateType& pluginState,
                        juce::ComboBox& combo,
                        juce::UndoManager* undoManager);

    ~ComboBoxAttachment() override;

    /** Sets the initial value of the box */
    void setValue (int newValue);

private:
    void comboBoxChanged (juce::ComboBox*) override;

    juce::ComboBox& comboBox;
    ParameterAttachment<ChoiceParameter,
                        PluginStateType,
                        ParameterAttachmentHelpers::SetValueCallback<ComboBoxAttachment>>
        attachment;
    juce::UndoManager* um = nullptr;

    bool skipBoxChangedCallback = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComboBoxAttachment)
};
} // namespace chowdsp

#include "chowdsp_ComboBoxAttachment.cpp"
