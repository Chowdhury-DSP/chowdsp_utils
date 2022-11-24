#pragma once

// @TODO: figure out how we want to handle these dependencies...
#include "../../../common/chowdsp_core/JUCEHelpers/juce_FixedSizeFunction.h"
#include "../../../dsp/chowdsp_dsp_data_structures/third_party/moodycamel/readerwriterqueue.h"

namespace chowdsp
{
#ifndef DOXYGEN
namespace plugin_state_detail
{
    template <typename ParamStateType, int count = 0, bool only_params = true, int index = pfr::tuple_size_v<ParamStateType>>
    struct ParamInfoHelper
    {
        template <typename T, bool isParam>
        struct SingleParamOrObjectInfo;

        template <typename T>
        struct SingleParamOrObjectInfo<T, true>
        {
            static constexpr int num_params = 1;
            static constexpr bool is_only_params = ParameterTypeHelpers::IsParameterPointerType<T>;
        };

        template <typename T>
        struct SingleParamOrObjectInfo<T, false>
        {
            static constexpr int num_params = ParamInfoHelper<T>::num_params;
            static constexpr bool is_only_params = ParamInfoHelper<T>::is_only_params;
        };

        using indexed_element_type = decltype (pfr::get<index - 1> (ParamStateType {}));
        static constexpr auto isParam = ParameterTypeHelpers::IsParameterPointerType<indexed_element_type>;

        static constexpr auto nextCount = count + SingleParamOrObjectInfo<indexed_element_type, isParam>::num_params;
        static constexpr int num_params = ParamInfoHelper<ParamStateType, nextCount, only_params, index - 1>::num_params;

        static constexpr auto nextOnlyParams = only_params & SingleParamOrObjectInfo<indexed_element_type, isParam>::is_only_params;
        static constexpr bool is_only_params = ParamInfoHelper<ParamStateType, count, nextOnlyParams, index - 1>::is_only_params;
    };

    template <typename ParamStateType, int count, bool only_params>
    struct ParamInfoHelper<ParamStateType, count, only_params, 0>
    {
        static constexpr int num_params = count;
        static constexpr bool is_only_params = only_params;
    };

    // @TODO: implement tests for these!!
    template <typename ParamStateType>
    static constexpr int ParamCount = ParamInfoHelper<ParamStateType>::num_params;

    template <typename ParamStateType>
    static constexpr int ContainsOnlyParamPointers = ParamInfoHelper<ParamStateType>::is_only_params;
} // namespace plugin_state_detail
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
                        [this] (auto& paramHolder, size_t index)
                        {
                            const auto* rangedParam = static_cast<juce::RangedAudioParameter*> (paramHolder.get());
                            paramInfoList[index] = ParamInfo { rangedParam, rangedParam->getValue() };
                        });

        startTimer (10); // @TODO: tune the timer interval
    }

    /** Constructs the state and adds all the state parameters to the given processor */
    explicit PluginState (juce::AudioProcessor& processor)
        : PluginState()
    {
        doForAllParams (params,
                        [&processor] (auto& paramHolder, size_t)
                        {
                            processor.addParameter (paramHolder.release());
                        });
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

    static constexpr auto totalNumParams = (size_t) plugin_state_detail::ParamCount<ParameterState>;

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
