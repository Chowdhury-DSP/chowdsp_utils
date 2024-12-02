namespace chowdsp
{
template <typename ParameterState, typename NonParameterState>
PluginStateImpl<ParameterState, NonParameterState>::PluginStateImpl (juce::UndoManager* um)
{
    initialise (params, nullptr, um);
}

template <typename ParameterState, typename NonParameterState>
PluginStateImpl<ParameterState, NonParameterState>::PluginStateImpl (juce::AudioProcessor& proc, juce::UndoManager* um)
{
    initialise (params, &proc, um);
}

template <typename ParameterState, typename NonParameterState>
PluginStateImpl<ParameterState, NonParameterState>::~PluginStateImpl()
{
    // Otherwise the listeners won't be deleted until after the parameters themselves.
    listeners.reset();
}

template <typename ParameterState, typename NonParameterState>
void PluginStateImpl<ParameterState, NonParameterState>::serialize (juce::MemoryBlock& data) const
{
    auto& arena = const_cast<ChainedArenaAllocator&> (*params.arena);
    const auto frame = arena.create_frame();

#if defined JucePlugin_VersionString
    serialize_object (currentPluginVersion.getVersionHint(), arena);
#else
    serialize_object (int {}, arena);
#endif

    NonParamState::serialize (arena, nonParams);
    ParamHolder::serialize (arena, params);

    dump_serialized_bytes (data, arena, &frame);
}

template <typename ParameterState, typename NonParameterState>
void PluginStateImpl<ParameterState, NonParameterState>::deserialize (juce::MemoryBlock&& dataBlock)
{
    callOnMainThread (
        [this, data = std::move (dataBlock)]
        {
            nonstd::span serial_data { static_cast<const std::byte*> (data.getData()), data.getSize() };
            if (serial_data.size() > 8
                && (static_cast<char> (serial_data[2]) == '['
                    || static_cast<char> (serial_data[2]) == '{'))
            {
                deserialize (JSONUtils::fromMemoryBlock (data), *this);
            }
            else
            {
                pluginStateVersion = Version::fromVersionHint (deserialize_object<int> (serial_data));
                NonParamState::deserialize (serial_data, nonParams, *params.arena);
                ParamHolder::deserialize (serial_data, params);
            }

            params.applyVersionStreaming (pluginStateVersion);
            if (nonParams.versionStreamingCallback != nullptr)
                nonParams.versionStreamingCallback (pluginStateVersion);

            getParameterListeners().updateBroadcastersFromMessageThread();

            if (undoManager != nullptr)
                undoManager->clearUndoHistory();
        });
}

/** Serializer */
template <typename ParameterState, typename NonParameterState>
json PluginStateImpl<ParameterState, NonParameterState>::serialize (const PluginStateImpl& object)
{
    return
    {
#if defined JucePlugin_VersionString
        { "version", currentPluginVersion },
#else
        { "version", chowdsp::Version {} },
#endif
            { "params", ParamHolder::serialize_json (object.params) },
            { "non-params", NonParamState::serialize_json (object.nonParams) },
    };
}

/** Legacy Deserializer */
template <typename ParameterState, typename NonParameterState>
void PluginStateImpl<ParameterState, NonParameterState>::legacy_deserialize (const json& serial, PluginStateImpl& object)
{
    using Serializer = JSONSerializer;
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
    Serializer::deserialize<Serializer> (Serializer::getChildElement (serial, versionChildIndex), object.pluginStateVersion);
#else
    using namespace version_literals;
    object.pluginStateVersion = "0.0.0"_v;
#endif

    NonParamState::legacy_deserialize (Serializer::getChildElement (serial, nonParamStateChildIndex), object.nonParams);
    ParamHolder::legacy_deserialize (Serializer::getChildElement (serial, paramStateChildIndex), object.params);
}

/** Deserializer */
template <typename ParameterState, typename NonParameterState>
void PluginStateImpl<ParameterState, NonParameterState>::deserialize (const json& serial, PluginStateImpl& object)
{
    if (serial.is_array())
    {
        legacy_deserialize (serial, object);
        return;
    }

    jassert (serial.find ("version") != serial.end());
    object.pluginStateVersion = serial.value ("version", Version {});

    NonParamState::deserialize_json (serial.at ("non-params"), object.nonParams);
    ParamHolder::deserialize_json (serial.at ("params"), object.params);
}

template <typename ParameterState, typename NonParameterState>
NonParamState& PluginStateImpl<ParameterState, NonParameterState>::getNonParameters()
{
    return nonParams;
}

template <typename ParameterState, typename NonParameterState>
const NonParamState& PluginStateImpl<ParameterState, NonParameterState>::getNonParameters() const
{
    return nonParams;
}
} // namespace chowdsp
