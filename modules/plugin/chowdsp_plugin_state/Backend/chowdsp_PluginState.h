#pragma once

// @TODO: figure out how we want to handle these dependencies...
#include "../../../common/chowdsp_core/JUCEHelpers/juce_FixedSizeFunction.h"
#include "../../../dsp/chowdsp_dsp_data_structures/third_party/moodycamel/readerwriterqueue.h"

namespace chowdsp
{
#ifndef DOXYGEN
namespace detail
{
    template <typename ParamStateType, int count = 0, int index = pfr::tuple_size_v<ParamStateType>>
    struct ParamCountHelper
    {
        static constexpr int nextCount = IsSmartPointer<decltype (pfr::get<index - 1> (ParamStateType {}))> ? count + 1 : count;
        static constexpr int value = ParamCountHelper<ParamStateType, index - 1, nextCount>::value;
    };

    template <typename ParamStateType, int count>
    struct ParamCountHelper<ParamStateType, 0, count>
    {
        static constexpr int value = count;
    };

    template <typename ParamStateType>
    static constexpr int ParamCount = ParamCountHelper<ParamStateType>::value;

    // @TODO: use doForParams instead?
    template <typename ParamsStateType>
    void addParamsToProcessor (juce::AudioProcessor& processor, ParamsStateType& paramsState)
    {
        pfr::for_each_field (paramsState,
                             [&processor] (auto& paramHolder)
                             {
                                 using Type = std::decay_t<decltype (paramHolder)>;
                                 if constexpr (IsSmartPointer<Type>)
                                 {
                                     using ParamType = typename std::decay_t<decltype (paramHolder)>::element_type;
                                     static_assert (std::is_base_of_v<juce::RangedAudioParameter, ParamType>,
                                                    "All parameters must be SmartPointers of JUCE parameter types!");

                                     processor.addParameter (paramHolder.release());
                                 }
                                 else
                                 {
                                     addParamsToProcessor (processor, paramHolder); // add nested params!
                                 }
                             });
    }
} // namespace detail
#endif

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
public:
    PluginState()
    {
        doForAllParams (params,
                        [this] (juce::RangedAudioParameter& param, size_t index)
                        {
                            paramInfoList[index] = ParamInfo { &param, param.getValue() };
                        });

        startTimer (10); // @TODO: tune the timer interval
    }

    /** Constructs the state and adds all the state parameters to the given processor */
    explicit PluginState (juce::AudioProcessor& processor)
        : PluginState()
    {
        detail::addParamsToProcessor (processor, params);
    }

    /** Serializes the plugin state to the given MemoryBlock */
    void serialize (juce::MemoryBlock& data)
    {
        Serialization::serialize<Serializer> (*this, data);
    }

    /** Deserializes the plugin state from the given MemoryBlock */
    void deserialize (const juce::MemoryBlock& data)
    {
        Serialization::deserialize<Serializer> (data, *this);
    }

    /** Serializer */
    template <typename S>
    static typename S::SerializedType serialize (const PluginState& object)
    {
        auto serial = S::createBaseElement();
        S::addChildElement (serial, ParameterStateSerializer::serialize<S> (object.params));
        S::addChildElement (serial, Serialization::serialize<S> (object.nonParams));
        return serial;
    }

    /** Deserializer */
    template <typename S>
    static void deserialize (typename S::DeserializedType serial, PluginState& object)
    {
        if (S::getNumChildElements (serial) != 2)
        {
            jassertfalse; // state load error!
            return;
        }

        ParameterStateSerializer::deserialize<S> (S::getChildElement (serial, 0), object.params);
        Serialization::deserialize<S> (S::getChildElement (serial, 1), object.nonParams);
    }

    /**
     * Adds a parameter listener which will be called on either the message
     * thread or the audio thread (you choose!). Listeners should have the
     * signature void().
     */
    template <typename ParamType, typename... ListenerArgs>
    auto addParameterListener (const ParamType& param, bool listenOnMessageThread, ListenerArgs&&... args)
    {
        const auto paramInfoIter = std::find_if (paramInfoList.begin(), paramInfoList.end(), [&param] (const ParamInfo& info)
                                                 { return info.paramCookie == &param; });

        if (paramInfoIter == paramInfoList.end())
        {
            jassertfalse; // trying to listen to a parameter that is not part of this state!
            return chowdsp::Callback {};
        }

        const auto index = (size_t) std::distance (paramInfoList.begin(), paramInfoIter);
        auto& broadcasterList = listenOnMessageThread ? messageThreadBroadcasters : audioThreadBroadcasters;
        return broadcasterList[index].connect (std::forward<ListenerArgs...> (args...));
    }

    /**
     * Runs parameter broadcasters synchronously.
     * This method is intended to be called from the audio thread.
     */
    void callAudioThreadBroadcasters()
    {
        AudioThreadAction action;
        while (audioThreadBroadcastQueue.try_dequeue (action))
            action();
    }

    ParameterState params;
    NonParameterState nonParams;

private:
    template <typename ParamsType, typename Callable>
    static constexpr size_t doForAllParams (ParamsType& params, Callable&& callable, size_t index = 0)
    {
        pfr::for_each_field (params,
                             [&index, call = std::forward<Callable> (callable)] (auto& paramHolder) mutable
                             {
                                 using Type = std::decay_t<decltype (paramHolder)>;
                                 if constexpr (IsSmartPointer<Type>)
                                 {
                                     call (*paramHolder, index++);
                                 }
                                 else
                                 {
                                     index = doForAllParams (paramHolder, std::forward<Callable> (call), index);
                                 }
                             });
        return index;
    }

    void callMessageThreadBroadcaster (size_t index)
    {
        messageThreadBroadcasters[index]();
    }

    void callAudioThreadBroadcaster (size_t index)
    {
        audioThreadBroadcasters[index]();
    }

    void hiResTimerCallback() override
    {
        for (const auto [index, paramInfo] : enumerate (paramInfoList))
        {
            if (paramInfo.paramCookie->getValue() == paramInfo.value)
                continue;

            paramInfo.value = paramInfo.paramCookie->getValue();
            messageThreadBroadcastQueue.enqueue ([this, i = index]
                                                 { callMessageThreadBroadcaster (i); });
            audioThreadBroadcastQueue.try_enqueue ([this, i = index]
                                                   { callAudioThreadBroadcaster (i); });
            triggerAsyncUpdate();
        }
    }

    void handleAsyncUpdate() override
    {
        MessageThreadAction action;
        while (messageThreadBroadcastQueue.try_dequeue (action))
            action();
    }

    struct ParamInfo
    {
        const juce::RangedAudioParameter* paramCookie = nullptr;
        float value = 0.0f;
    };

    // @TODO: figure out how to count parameters at compile-time!
    static constexpr auto totalNumParams = (size_t) detail::ParamCount<ParameterState>;

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
