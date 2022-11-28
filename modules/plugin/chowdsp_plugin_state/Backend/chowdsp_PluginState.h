#pragma once

// @TODO: figure out how we want to handle these dependencies...
#include "../../../common/chowdsp_core/JUCEHelpers/juce_FixedSizeFunction.h"
#include "../../../dsp/chowdsp_dsp_data_structures/third_party/moodycamel/readerwriterqueue.h"

namespace chowdsp
{

struct NullState
{
};

/**
 * Template type to hold a plugin's state.
 *
 * @tparam ParameterState       Struct containing all of the plugin's parameters as chowdsp::SmartPointer's. Structs can be nested if necessary.
 * @tparam NonParameterState    Struct containing all of the plugin's non-parameter state.
 * @tparam Serializer           A type that implements chowdsp::BaseSerializer (JSONSerializer by default)
 */
template <typename ParameterState, typename NonParameterState = NullState, typename Serializer = JSONSerializer>
class PluginState : private juce::HighResolutionTimer,
                    private juce::AsyncUpdater
{
    static_assert (PluginStateHelpers::ContainsOnlyParamPointers<ParameterState>,
                   "ParameterState must contain only chowdsp::SmartPointer<> of parameter types,"
                   " or structs containing those types!");

public:
    /** Constructs a plugin state with no processor */
    explicit PluginState (juce::UndoManager* um = nullptr);

    /** Constructs the state and adds all the state parameters to the given processor */
    explicit PluginState (juce::AudioProcessor& processor, juce::UndoManager* um = nullptr);

    /** Serializes the plugin state to the given MemoryBlock */
    void serialize (juce::MemoryBlock& data) const;

    /** Deserializes the plugin state from the given MemoryBlock */
    void deserialize (const juce::MemoryBlock& data);

    /** Serializer */
    template <typename>
    static typename Serializer::SerializedType serialize (const PluginState& object);

    /** Deserializer */
    template <typename>
    static void deserialize (typename Serializer::DeserializedType serial, PluginState& object);

    /**
     * Adds a parameter listener which will be called on either the message
     * thread or the audio thread (you choose!). Listeners should have the
     * signature void().
     */
    template <typename ParamType, typename... ListenerArgs>
    auto addParameterListener (const ParamType& param, bool listenOnMessageThread, ListenerArgs&&... args);

    /**
     * Runs parameter broadcasters synchronously.
     * This method is intended to be called from the audio thread.
     */
    void callAudioThreadBroadcasters();

    /**
     * Runs parameter broadcasters synchronously.
     * This method is intended to be called from the message thread.
     */
    void callMessageThreadBroadcasters();

    ParameterState params;
    NonParameterState nonParams;
    juce::UndoManager* undoManager = nullptr;

private:
    template <typename ParamsType, typename Callable>
    static constexpr size_t doForAllParams (ParamsType& params, Callable&& callable, size_t index = 0)
    {
        pfr::for_each_field (params,
                             [&index, call = std::forward<Callable> (callable)] (auto& paramHolder) mutable
                             {
                                 using Type = std::decay_t<decltype (paramHolder)>;
                                 if constexpr (ParameterTypeHelpers::IsParameterPointerType<Type>)
                                 {
                                     call (paramHolder, index++);
                                 }
                                 else
                                 {
                                     index = doForAllParams (paramHolder, std::forward<Callable> (call), index);
                                 }
                             });
        return index;
    }

    void callMessageThreadBroadcaster (size_t index);
    void callAudioThreadBroadcaster (size_t index);

    void hiResTimerCallback() override;
    void handleAsyncUpdate() override;

    struct ParamInfo
    {
        const juce::RangedAudioParameter* paramCookie = nullptr;
        float value = 0.0f;
    };

    static constexpr auto totalNumParams = (size_t) PluginStateHelpers::ParamCount<ParameterState>;

    std::array<ParamInfo, totalNumParams> paramInfoList;

    std::array<chowdsp::Broadcaster<void()>, totalNumParams> messageThreadBroadcasters;
    using MessageThreadAction = juce::dsp::FixedSizeFunction<sizeof (&PluginState::callMessageThreadBroadcaster), void()>;
    moodycamel::ReaderWriterQueue<MessageThreadAction> messageThreadBroadcastQueue { totalNumParams };

    std::array<chowdsp::Broadcaster<void()>, totalNumParams> audioThreadBroadcasters;
    using AudioThreadAction = juce::dsp::FixedSizeFunction<sizeof (&PluginState::callAudioThreadBroadcaster), void()>;
    moodycamel::ReaderWriterQueue<AudioThreadAction> audioThreadBroadcastQueue { totalNumParams };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginState)
};
} // namespace chowdsp

#include "chowdsp_PluginState.cpp"
