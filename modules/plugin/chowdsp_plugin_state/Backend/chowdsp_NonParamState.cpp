namespace chowdsp
{
inline void NonParamState::addStateValues (const std::initializer_list<StateValueBase*>& newStateValues)
{
    values.insert (values.end(), newStateValues.begin(), newStateValues.end());
    validateStateValues();
}

template <typename T>
inline void NonParamState::addStateValues (nonstd::span<StateValue<T>> newStateValues)
{
    for (auto& val : newStateValues)
        values.push_back (&val);
    validateStateValues();
}

inline void NonParamState::validateStateValues() const
{
    std::vector<std::string_view> stateValueNames;
    for (const auto& value : values)
    {
        // Duplicate state value names are not allowed!
        jassert (std::find (stateValueNames.begin(), stateValueNames.end(), value->name) == stateValueNames.end());
        stateValueNames.emplace_back (value->name);
    }

    jassert (stateValueNames.size() == values.size()); // something has gone horrible wrong!
}

template <typename Serializer>
typename Serializer::SerializedType NonParamState::serialize (const NonParamState& state)
{
    auto serial = Serializer::createBaseElement();
    for (const auto& value : state.values)
        value->serialize (serial);
    return serial;
}

template <typename Serializer>
void NonParamState::deserialize (typename Serializer::DeserializedType deserial, const NonParamState& state)
{
    juce::StringArray namesThatHaveBeenDeserialized {};
    if (const auto numNamesAndVals = Serializer::getNumChildElements (deserial); numNamesAndVals % 2 == 0)
    {
        for (int i = 0; i < numNamesAndVals; i += 2)
        {
            juce::String name {};
            Serialization::deserialize<Serializer> (Serializer::getChildElement (deserial, i), name);
            const auto valueDeserial = Serializer::getChildElement (deserial, i + 1);
            for (auto& value : state.values)
            {
                if (name == toString (value->name))
                {
                    value->deserialize (valueDeserial);
                    namesThatHaveBeenDeserialized.add (name);
                }
            }
        }
    }
    else
    {
        jassertfalse; // state loading error
    }

    // set all un-matched objects to their default values
    for (auto& value : state.values)
    {
        if (! namesThatHaveBeenDeserialized.contains (toString (value->name)))
            value->reset();
    }
}
} // namespace chowdsp
