namespace chowdsp
{
template <typename ParameterState, typename NonParameterState, typename Serializer>
PluginStateImpl<ParameterState, NonParameterState, Serializer>::PluginStateImpl (juce::UndoManager* um)
{
    initialise (params, nullptr, um);
}

template <typename ParameterState, typename NonParameterState, typename Serializer>
PluginStateImpl<ParameterState, NonParameterState, Serializer>::PluginStateImpl (juce::AudioProcessor& proc, juce::UndoManager* um)
{
    initialise (params, &proc, um);
}

template <typename ParameterState, typename NonParameterState, typename Serializer>
void PluginStateImpl<ParameterState, NonParameterState, Serializer>::serialize (juce::MemoryBlock& data) const
{
    Serialization::serialize<Serializer> (*this, data);
}

template <typename ParameterState, typename NonParameterState, typename Serializer>
void PluginStateImpl<ParameterState, NonParameterState, Serializer>::deserialize (const juce::MemoryBlock& data)
{
    callOnMainThread (
        [this, data]
        {
            Serialization::deserialize<Serializer> (data, *this);

            params.applyVersionStreaming (pluginStateVersion);
            if (nonParams.versionStreamingCallback != nullptr)
                nonParams.versionStreamingCallback (pluginStateVersion);

            getParameterListeners().updateBroadcastersFromMessageThread();

            if (undoManager != nullptr)
                undoManager->clearUndoHistory();
        });
}

/** Serializer */
template <typename ParameterState, typename NonParameterState, typename Serializer>
template <typename>
typename Serializer::SerializedType PluginStateImpl<ParameterState, NonParameterState, Serializer>::serialize (const PluginStateImpl& object)
{
    auto serial = Serializer::createBaseElement();

#if defined JucePlugin_VersionString
    Serializer::addChildElement (serial, Serializer::template serialize<Serializer> (currentPluginVersion));
#endif

    Serializer::addChildElement (serial, Serializer::template serialize<Serializer, NonParamState> (object.nonParams));
    Serializer::addChildElement (serial, Serializer::template serialize<Serializer, ParamHolder> (object.params));
    return serial;
}

/** Deserializer */
template <typename ParameterState, typename NonParameterState, typename Serializer>
template <typename>
void PluginStateImpl<ParameterState, NonParameterState, Serializer>::deserialize (typename Serializer::DeserializedType serial, PluginStateImpl& object)
{
    enum
    {
#if defined JucePlugin_VersionString
        versionChildIndex,
#endif
        nonParamStateChildIndex,
        paramStateChildIndex,
        expectedNumChildElements,
    };

    if (Serializer::getNumChildElements (serial) != expectedNumChildElements)
    {
        jassertfalse; // state load error!
        return;
    }

#if defined JucePlugin_VersionString
    Serializer::template deserialize<Serializer> (Serializer::getChildElement (serial, versionChildIndex), object.pluginStateVersion);
#else
    using namespace version_literals;
    object.pluginStateVersion = "0.0.0"_v;
#endif

    Serializer::template deserialize<Serializer, NonParamState> (Serializer::getChildElement (serial, nonParamStateChildIndex), object.nonParams);
    Serializer::template deserialize<Serializer, ParamHolder> (Serializer::getChildElement (serial, paramStateChildIndex), object.params);
}

template <typename ParameterState, typename NonParameterState, typename Serializer>
NonParamState& PluginStateImpl<ParameterState, NonParameterState, Serializer>::getNonParameters()
{
    return nonParams;
}

template <typename ParameterState, typename NonParameterState, typename Serializer>
const NonParamState& PluginStateImpl<ParameterState, NonParameterState, Serializer>::getNonParameters() const
{
    return nonParams;
}
} // namespace chowdsp
