namespace chowdsp
{
template <typename ParameterState, typename NonParameterState, typename Serializer>
PluginState<ParameterState, NonParameterState, Serializer>::PluginState (juce::UndoManager* um)
    : undoManager (um)
{
    doForAllParams (params,
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
    doForAllParams (params,
                    [&processor] (auto& paramHolder, size_t)
                    {
                        processor.addParameter (paramHolder.release());
                    });
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
    Serializer::addChildElement (serial, ParameterStateSerializer::serialize<Serializer> (object.params));
    Serializer::addChildElement (serial, Serialization::serialize<Serializer> (object.nonParams));
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

    ParameterStateSerializer::deserialize<Serializer> (Serializer::getChildElement (serial, 0), object.params);
    Serialization::deserialize<Serializer> (Serializer::getChildElement (serial, 1), object.nonParams);
}

template <typename ParameterState, typename NonParameterState, typename Serializer>
template <typename ParamType, typename... ListenerArgs>
auto PluginState<ParameterState, NonParameterState, Serializer>::addParameterListener (const ParamType& param, bool listenOnMessageThread, ListenerArgs&&... args)
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

template <typename ParameterState, typename NonParameterState, typename Serializer>
void PluginState<ParameterState, NonParameterState, Serializer>::callAudioThreadBroadcasters()
{
    AudioThreadAction action;
    while (audioThreadBroadcastQueue.try_dequeue (action))
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
    MessageThreadAction action;
    while (messageThreadBroadcastQueue.try_dequeue (action))
        action();
}
} // namespace chowdsp
