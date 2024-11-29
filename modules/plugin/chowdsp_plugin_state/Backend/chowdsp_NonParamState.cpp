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

template <typename ContainerType>
inline void NonParamState::addStateValues (ContainerType& container)
{
    for (auto& val : container)
        values.push_back (&val);
    validateStateValues();
}

inline void NonParamState::validateStateValues() const
{
#if JUCE_DEBUG
    std::vector<std::string_view> stateValueNames;
    for (const auto& value : values)
    {
        // State value name must not be empty
        jassert (! value->name.empty());

        // Duplicate state value names are not allowed!
        jassert (std::find (stateValueNames.begin(), stateValueNames.end(), value->name) == stateValueNames.end());

        stateValueNames.emplace_back (value->name);
    }

    jassert (stateValueNames.size() == values.size()); // something has gone horrible wrong!
#endif
}

inline json NonParamState::serialize (const NonParamState& state)
{
    auto serial = nlohmann::json::object();
    for (const auto& value : state.values)
        serial[value->name] = value->serialize();
    return serial;
}

inline void NonParamState::legacy_deserialize (const json& deserial, const NonParamState& state)
{
    using Serializer = JSONSerializer;
    std::vector<std::string_view> namesThatHaveBeenDeserialized {};
    if (const auto numNamesAndVals = Serializer::getNumChildElements (deserial); numNamesAndVals % 2 == 0)
    {
        namesThatHaveBeenDeserialized.reserve (static_cast<size_t> (numNamesAndVals) / 2);
        for (int i = 0; i < numNamesAndVals; i += 2)
        {
            const auto name = Serializer::getChildElement (deserial, i).get<std::string_view>();
            const auto& valueDeserial = Serializer::getChildElement (deserial, i + 1);
            for (auto& value : state.values)
            {
                if (name == value->name)
                {
                    value->deserialize (valueDeserial);
                    namesThatHaveBeenDeserialized.push_back (name);
                }
            }
        }
    }
    else
    {
        jassertfalse; // state loading error
    }

    // set all un-matched objects to their default values
    if (! namesThatHaveBeenDeserialized.empty())
    {
        for (auto& value : state.values)
        {
            if (std::find (namesThatHaveBeenDeserialized.begin(), namesThatHaveBeenDeserialized.end(), value->name) == namesThatHaveBeenDeserialized.end())
                value->reset();
        }
    }
}

inline void NonParamState::deserialize (const json& deserial, const NonParamState& state)
{
    for (auto& value : state.values)
    {
        auto iter = deserial.find (value->name);
        if (iter != deserial.end())
            value->deserialize (*iter);
        else
            value->reset();
    }
}
} // namespace chowdsp
