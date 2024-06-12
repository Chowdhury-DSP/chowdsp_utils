#pragma once

#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_utils
#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>
#else
#include "../../chowdsp_plugin_utils/Threads/chowdsp_DeferredMainThreadAction.h"
#endif

namespace chowdsp
{
/** Base class for managing a plugin's state. */
class PluginState
{
public:
    PluginState() = default;
    virtual ~PluginState() = default;

    /** Initialises the plugin state with a given set of parameters. */
    void initialise (ParamHolder& parameters,
                     juce::AudioProcessor* proc = nullptr,
                     juce::UndoManager* um = nullptr)
    {
        params = &parameters;
        processor = proc;
        undoManager = um;
        if (processor != nullptr)
            parameters.connectParametersToProcessor (*processor);
        listeners.emplace (parameters, processor);
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
    template <typename... ListenerArgs>
    [[nodiscard]] ScopedCallback addNonParameterListener (StateValueBase& stateValue,
                                                          ListenerArgs&&... args) const
    {
        return stateValue.changeBroadcaster.connect (std::forward<ListenerArgs...> (args...));
    }

    /** Returns the plugin state's parameters */
    [[nodiscard]] ParamHolder& getParameters() { return *params; }

    /** Returns the plugin state's parameters */
    [[nodiscard]] const ParamHolder& getParameters() const { return *params; }

    /** Returns the plugin non-parameter state */
    [[nodiscard]] virtual NonParamState& getNonParameters() = 0;

    /** Returns the plugin non-parameter state */
    [[nodiscard]] virtual const NonParamState& getNonParameters() const = 0;

    /** Calls an action on the main thread via chowdsp::DeferredAction */
    template <typename Callable>
    void callOnMainThread (Callable&& func, bool couldBeAudioThread = false)
    {
        mainThreadAction.call (std::forward<Callable> (func), couldBeAudioThread);
    }

    juce::AudioProcessor* processor = nullptr;
    juce::UndoManager* undoManager = nullptr;

private:
    std::optional<ParameterListeners> listeners;
    ParamHolder* params = nullptr;
    DeferredAction mainThreadAction;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginState)
};

/** A "dummy" plugin state that does absolutely nothing! */
struct DummyPluginState : PluginState
{
    void serialize (juce::MemoryBlock&) const override {}
    void deserialize (const juce::MemoryBlock&) override {}

    NonParamState non_params {};
    [[nodiscard]] NonParamState& getNonParameters() override { return non_params; }
    [[nodiscard]] const NonParamState& getNonParameters() const override { return non_params; }
};
} // namespace chowdsp
