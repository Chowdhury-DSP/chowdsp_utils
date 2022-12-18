#pragma once

#include <chowdsp_plugin_state/chowdsp_plugin_state.h>

namespace chowdsp
{
/** Clone of juce::GenericAudioProcessorEditor, but usable as a generic component */
class ParametersView : public juce::Component
{
public:
    explicit ParametersView (PluginState& pluginState, ParamHolder& params);
    ~ParametersView() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParametersView)
};

/** Clone of juce::GenericAudioProcessorEditor. */
class ParametersViewEditor : public juce::AudioProcessorEditor
{
public:
    ParametersViewEditor (juce::AudioProcessor& proc, PluginState& pluginState, ParamHolder& params)
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
    ParametersView view;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParametersViewEditor)
};
} // namespace chowdsp
