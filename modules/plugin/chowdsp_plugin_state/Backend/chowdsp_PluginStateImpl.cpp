namespace chowdsp
{
template <typename ParameterState, typename NonParameterState, typename Serializer>
PluginStateImpl<ParameterState, NonParameterState, Serializer>::PluginStateImpl (juce::UndoManager* um)
{
    initialise (params, nullptr, um);
}

template <typename ParameterState, typename NonParameterState, typename Serializer>
PluginStateImpl<ParameterState, NonParameterState, Serializer>::PluginStateImpl (juce::AudioProcessor& processor, juce::UndoManager* um)
{
    initialise (params, &processor, um);
}

template <typename ParameterState, typename NonParameterState, typename Serializer>
void PluginStateImpl<ParameterState, NonParameterState, Serializer>::serialize (juce::MemoryBlock& data) const
{
    Serialization::serialize<Serializer> (*this, data);
}

template <typename ParameterState, typename NonParameterState, typename Serializer>
void PluginStateImpl<ParameterState, NonParameterState, Serializer>::deserialize (const juce::MemoryBlock& data)
{
    Serialization::deserialize<Serializer> (data, *this);

    if (undoManager != nullptr)
        undoManager->clearUndoHistory();
}

/** Serializer */
template <typename ParameterState, typename NonParameterState, typename Serializer>
template <typename>
typename Serializer::SerializedType PluginStateImpl<ParameterState, NonParameterState, Serializer>::serialize (const PluginStateImpl& object)
{
    auto serial = Serializer::createBaseElement();
    Serializer::addChildElement (serial, Serializer::template serialize<Serializer, ParamHolder> (object.params));
    Serializer::addChildElement (serial, Serializer::template serialize<Serializer, NonParamState> (object.nonParams));
    return serial;
}

/** Deserializer */
template <typename ParameterState, typename NonParameterState, typename Serializer>
template <typename>
void PluginStateImpl<ParameterState, NonParameterState, Serializer>::deserialize (typename Serializer::DeserializedType serial, PluginStateImpl& object)
{
    // @TODO: What about version streaming?

    if (Serializer::getNumChildElements (serial) != 2)
    {
        jassertfalse; // state load error!
        return;
    }

    Serializer::template deserialize<Serializer, ParamHolder> (Serializer::getChildElement (serial, 0), object.params);
    Serializer::template deserialize<Serializer, NonParamState> (Serializer::getChildElement (serial, 1), object.nonParams);
}
} // namespace chowdsp
