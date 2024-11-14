#pragma once

#include <chowdsp_plugin_state/chowdsp_plugin_state.h>

namespace chowdsp
{
/** Clone of juce::GenericAudioProcessorEditor, but usable as a generic component */
class ParametersView : public juce::Component
{
public:
    ParametersView (PluginState& pluginState, ParamHolder& params);
    ParametersView (ParameterListeners& paramListeners, ParamHolder& params);
    ~ParametersView() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    /** Returns nullptr if no component is found for the given parameter */
    [[nodiscard]] juce::Component* getComponentForParameter (const juce::RangedAudioParameter&);

private:
    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    juce::String versionInfoText {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParametersView)
};

/** Clone of juce::GenericAudioProcessorEditor. */
class ParametersViewEditor : public juce::AudioProcessorEditor
{
public:
    template <typename PluginType>
    explicit ParametersViewEditor (PluginType& plugin)
        : ParametersViewEditor (plugin, plugin.getState(), plugin.getState().params)
    {
    }

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

    ParametersView view;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParametersViewEditor)
};
} // namespace chowdsp
