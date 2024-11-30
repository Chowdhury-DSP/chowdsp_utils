namespace chowdsp
{
void NonParamState::addStateValues (const std::initializer_list<StateValueBase*>& newStateValues)
{
    values.insert (values.end(), newStateValues.begin(), newStateValues.end());
    validateStateValues();
}

void NonParamState::validateStateValues() const
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

void NonParamState::reset()
{
    for (auto* value : values)
        value->reset();
}

void NonParamState::serialize (ChainedArenaAllocator& arena, const NonParamState& state)
{
    auto* serialize_num_bytes = arena.allocate<bytes_detail::size_type> (1, 1);
    size_t num_bytes = 0;
    for (const auto& value : state.values)
    {
        num_bytes += serialize_string (value->name, arena);
        num_bytes += value->serialize (arena);
    }
    serialize_direct (serialize_num_bytes, num_bytes);
}

void NonParamState::deserialize (nonstd::span<const std::byte> serial_data, NonParamState& state)
{
    auto num_bytes = deserialize_direct<bytes_detail::size_type> (serial_data);
    if (num_bytes == 0 || serial_data.size() < num_bytes)
    {
        state.reset();
        return;
    }

    auto values_copy = state.values;
    auto values_iter = values_copy.begin();
    const auto get_value_ptr = [&] (std::string_view name) -> StateValueBase*
    {
        for (auto iter = values_iter; iter != values_copy.end(); ++iter)
        {
            if ((*iter)->name == name)
            {
                auto* ptr = *iter;
                values_iter = values_copy.erase (iter);
                return ptr;
            }
        }

        for (auto iter = values_copy.begin(); iter != values_iter; ++iter)
        {
            if ((*iter)->name == name)
            {
                auto* ptr = *iter;
                values_iter = values_copy.erase (iter);
                return ptr;
            }
        }
        return nullptr;
    };

    while (serial_data.size() > 0)
    {
        const auto value_name = deserialize_string (serial_data);
        auto* value = get_value_ptr (value_name);
        if (value == nullptr)
        {
            const auto value_num_bytes = deserialize_direct<bytes_detail::size_type> (serial_data);
            serial_data = serial_data.subspan (value_num_bytes);
            continue;
        }

        value->deserialize (serial_data);
    }

    for (auto* value : values_copy)
        value->reset();
}

json NonParamState::serialize_json (const NonParamState& state)
{
    auto serial = nlohmann::json::object();
    for (const auto& value : state.values)
        serial[value->name] = value->serialize();
    return serial;
}

void NonParamState::legacy_deserialize (const json& deserial, const NonParamState& state)
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

void NonParamState::deserialize_json (const json& deserial, const NonParamState& state)
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
