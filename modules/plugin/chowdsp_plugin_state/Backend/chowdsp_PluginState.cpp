namespace chowdsp
{
template <typename ParameterState, typename NonParameterState, typename Serializer>
PluginState<ParameterState, NonParameterState, Serializer>::PluginState (juce::UndoManager* um)
    : undoManager (um)
{
    PluginStateHelpers::doForAllFields (params,
                                        [this] (auto& paramHolder, size_t index)
                                        {
                                            const auto* rangedParam = static_cast<juce::RangedAudioParameter*> (paramHolder.get());
                                            paramInfoList[index] = ParamInfo { rangedParam, rangedParam->getValue() };
                                        });

    startTimer (10); // @TODO: tune the timer interval
}

template <typename ParameterState, typename NonParameterState, typename Serializer>
PluginState<ParameterState, NonParameterState, Serializer>::PluginState (juce::AudioProcessor& processor, juce::UndoManager* um)
    : PluginState (um)
{
    PluginStateHelpers::doForAllFields (params,
                                        [&processor] (auto& paramHolder, size_t)
                                        {
                                            // Parameter must be non-null and owned by it's pointer before being released to the processor
                                            jassert (paramHolder != nullptr && paramHolder.isOwner());
                                            processor.addParameter (paramHolder.release());
                                        });
}

template <typename ParameterState, typename NonParameterState, typename Serializer>
PluginState<ParameterState, NonParameterState, Serializer>::~PluginState()
{
    stopTimer();
}

template <typename ParameterState, typename NonParameterState, typename Serializer>
template <typename ParameterType>
ParameterType& PluginState<ParameterState, NonParameterState, Serializer>::getParameter (const ParameterPath& path)
{
    auto* paramPtr = ParameterPath::getParameterForPath<ParameterType> (params, path);
    jassert (paramPtr != nullptr); // Unable to find parameter at this path!
    return *paramPtr;
}

template <typename ParameterState, typename NonParameterState, typename Serializer>
template <typename ParameterType>
const ParameterType& PluginState<ParameterState, NonParameterState, Serializer>::getParameter (const ParameterPath& path) const
{
    return const_cast<PluginState*> (this)->getParameter<ParameterType> (path);
}

template <typename ParameterState, typename NonParameterState, typename Serializer>
void PluginState<ParameterState, NonParameterState, Serializer>::serialize (juce::MemoryBlock& data) const
{
    Serialization::serialize<Serializer> (*this, data);
}

template <typename ParameterState, typename NonParameterState, typename Serializer>
void PluginState<ParameterState, NonParameterState, Serializer>::deserialize (const juce::MemoryBlock& data)
{
    Serialization::deserialize<Serializer> (data, *this);

    if (undoManager != nullptr)
        undoManager->clearUndoHistory();
}

/** Serializer */
template <typename ParameterState, typename NonParameterState, typename Serializer>
template <typename>
typename Serializer::SerializedType PluginState<ParameterState, NonParameterState, Serializer>::serialize (const PluginState& object)
{
    auto serial = Serializer::createBaseElement();
    Serializer::addChildElement (serial, PluginStateSerializer::serialize<Serializer> (object.params));
    Serializer::addChildElement (serial, PluginStateSerializer::serialize<Serializer> (object.nonParams));
    return serial;
}

/** Deserializer */
template <typename ParameterState, typename NonParameterState, typename Serializer>
template <typename>
void PluginState<ParameterState, NonParameterState, Serializer>::deserialize (typename Serializer::DeserializedType serial, PluginState& object)
{
    if (Serializer::getNumChildElements (serial) != 2)
    {
        jassertfalse; // state load error!
        return;
    }

    PluginStateSerializer::deserialize<Serializer> (Serializer::getChildElement (serial, 0), object.params);
    PluginStateSerializer::deserialize<Serializer> (Serializer::getChildElement (serial, 1), object.nonParams);
}

template <typename ParameterState, typename NonParameterState, typename Serializer>
template <typename ParamType, typename... ListenerArgs>
ScopedCallback PluginState<ParameterState, NonParameterState, Serializer>::addParameterListener (const ParamType& param, bool listenOnMessageThread, ListenerArgs&&... args)
{
    const auto paramInfoIter = std::find_if (paramInfoList.begin(), paramInfoList.end(), [&param] (const ParamInfo& info)
                                             { return info.paramCookie == &param; });

    if (paramInfoIter == paramInfoList.end())
    {
        jassertfalse; // trying to listen to a parameter that is not part of this state!
        return {};
    }

    const auto index = (size_t) std::distance (paramInfoList.begin(), paramInfoIter);
    auto& broadcasterList = listenOnMessageThread ? messageThreadBroadcasters : audioThreadBroadcasters;
    return broadcasterList[index].connect (std::forward<ListenerArgs...> (args...));
}

template <typename ParameterState, typename NonParameterState, typename Serializer>
template <typename NonParamType, typename... ListenerArgs>
ScopedCallback PluginState<ParameterState, NonParameterState, Serializer>::addNonParameterListener (StateValue<NonParamType>& nonParam, ListenerArgs&&... args)
{
    return nonParam.changeBroadcaster.connect (std::forward<ListenerArgs...> (args...));
}

template <typename ParameterState, typename NonParameterState, typename Serializer>
void PluginState<ParameterState, NonParameterState, Serializer>::callAudioThreadBroadcasters()
{
    AudioThreadAction action;
    while (audioThreadBroadcastQueue.try_dequeue (action))
        action();
}

template <typename ParameterState, typename NonParameterState, typename Serializer>
void PluginState<ParameterState, NonParameterState, Serializer>::callMessageThreadBroadcasters()
{
    MessageThreadAction action;
    while (messageThreadBroadcastQueue.try_dequeue (action))
        action();
}

template <typename ParameterState, typename NonParameterState, typename Serializer>
void PluginState<ParameterState, NonParameterState, Serializer>::callMessageThreadBroadcaster (size_t index)
{
    messageThreadBroadcasters[index]();
}

template <typename ParameterState, typename NonParameterState, typename Serializer>
void PluginState<ParameterState, NonParameterState, Serializer>::callAudioThreadBroadcaster (size_t index)
{
    audioThreadBroadcasters[index]();
}

template <typename ParameterState, typename NonParameterState, typename Serializer>
void PluginState<ParameterState, NonParameterState, Serializer>::hiResTimerCallback()
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

template <typename ParameterState, typename NonParameterState, typename Serializer>
void PluginState<ParameterState, NonParameterState, Serializer>::handleAsyncUpdate()
{
    callMessageThreadBroadcasters();
}
} // namespace chowdsp
