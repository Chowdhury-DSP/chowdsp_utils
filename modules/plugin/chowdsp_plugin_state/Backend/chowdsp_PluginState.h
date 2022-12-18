#pragma once

namespace chowdsp
{
class PluginState
{
public:
    PluginState() = default;

    void initialise (ParamHolder& parameters,
                     juce::AudioProcessor* processor = nullptr,
                     juce::UndoManager* um = nullptr,
                     juce::TimeSliceThread* backgroundThread = nullptr)
    {
        undoManager = um;
        listeners.emplace (parameters, backgroundThread);
        if (processor != nullptr)
            parameters.connectParametersToProcessor (*processor);
    }

    /** Serializes the plugin state to the given MemoryBlock */
    virtual void serialize (juce::MemoryBlock& data) const = 0;

    /** Deserializes the plugin state from the given MemoryBlock */
    virtual void deserialize (const juce::MemoryBlock& data) = 0;

    /**
     * Adds a parameter listener which will be called on either the message
     * thread or the audio thread (you choose!). Listeners should have the
     * signature void().
     */
    template <typename... ListenerArgs>
    [[nodiscard]] ScopedCallback addParameterListener (const juce::RangedAudioParameter& param,
                                                       ParameterListenerThread listenerThread,
                                                       ListenerArgs&&... args)
    {
        return listeners->addParameterListener (param, listenerThread, std::forward<ListenerArgs...> (args...));
    }

    /** Returns the plugin parameter listeners. */
    auto& getParameterListeners() { return *listeners; }

    /**
     * Adds a listener to some field of the plugin's non-parameter state.
     * The listener will be called on whichever thread the state value is
     * mutated on. Listeners should have the signature void().
     */
    //    template <typename NonParamType, typename... ListenerArgs>
    //    [[nodiscard]] ScopedCallback addNonParameterListener (StateValue<NonParamType>& nonParam, ListenerArgs&&... args)
    //    {
    //        return nonParam.changeBroadcaster.connect (std::forward<ListenerArgs...> (args...));
    //    }

    juce::UndoManager* undoManager = nullptr;

private:
    std::optional<ParameterListeners> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginState)
};
} // namespace chowdsp
