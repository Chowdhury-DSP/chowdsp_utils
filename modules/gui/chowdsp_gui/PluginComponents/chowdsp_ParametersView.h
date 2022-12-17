#pragma once

#include <chowdsp_plugin_state/chowdsp_plugin_state.h>

namespace chowdsp
{
/** Clone of juce::GenericAudioProcessorEditor, but usable as a generic component */
template <typename PluginStateType>
class ParametersView : public juce::Component
{
public:
    explicit ParametersView (PluginStateType& pluginState, ParamHolder& params);
    ~ParametersView() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParametersView)
};

/** Clone of juce::GenericAudioProcessorEditor. */
template <typename PluginStateType>
class ParametersViewEditor : public juce::AudioProcessorEditor
{
public:
    ParametersViewEditor (juce::AudioProcessor& proc, PluginStateType& pluginState, ParamHolder& params)
        : juce::AudioProcessorEditor (proc),
          view (pluginState, params)
    {
        setResizable (true, false);
        setSize (view.getWidth(), view.getHeight());

        addAndMakeVisible (view);
    }

    void resized() override
    {
        view.setBounds (getLocalBounds());
    }

private:
    ParametersView<PluginStateType> view;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParametersViewEditor)
};
} // namespace chowdsp

#include "chowdsp_ParametersView.cpp"
