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

void NonParamState::deserialize (nonstd::span<const std::byte>& serial_data, NonParamState& state, ChainedArenaAllocator& arena)
{
    auto num_bytes = deserialize_direct<bytes_detail::size_type> (serial_data);
    if (num_bytes == 0)
    {
        state.reset();
        return;
    }

    auto data = serial_data.subspan (0, num_bytes);
    serial_data = serial_data.subspan (num_bytes);

    const auto _ = arena.create_frame();
    auto values_copy = arena::make_span<StateValueBase*> (arena, state.values.size());
    std::copy (state.values.begin(), state.values.end(), values_copy.begin());
    auto values_iter = values_copy.begin();
    size_t counter = 0;
    const auto get_value_ptr = [&] (std::string_view name) -> StateValueBase*
    {
        const auto returner = [&] (auto& iter)
        {
            auto* ptr = *iter;
            *iter = nullptr;
            ++iter;
            values_iter = iter;
            counter++;
            return ptr;
        };

        for (auto iter = values_iter; iter != values_copy.end(); ++iter)
        {
            if (*iter != nullptr && (*iter)->name == name)
                return returner (iter);
        }
        for (auto iter = values_copy.begin(); iter != values_iter; ++iter)
        {
            if (*iter != nullptr && (*iter)->name == name)
                return returner (iter);
        }
        return nullptr;
    };

    while (! data.empty())
    {
        const auto value_name = deserialize_string (data);
        auto* value = get_value_ptr (value_name);
        if (value == nullptr)
        {
            const auto value_num_bytes = deserialize_direct<bytes_detail::size_type> (data);
            data = data.subspan (value_num_bytes);
            continue;
        }

        value->deserialize (data);
    }

    if (counter < values_copy.size())
    {
        for (auto* value : values_copy)
            if (value != nullptr)
                value->reset();
    }
}

json NonParamState::serialize_json (const NonParamState& state)
{
    auto serial = nlohmann::json::object();
    for (const auto& value : state.values)
        serial[value->name] = value->serialize_json();
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
                    value->deserialize_json (valueDeserial);
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
            value->deserialize_json (*iter);
        else
            value->reset();
    }
}
} // namespace chowdsp
