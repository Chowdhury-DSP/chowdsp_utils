#pragma once

namespace chowdsp
{
/** Container for holding non-parameter state values. */
class NonParamState
{
public:
    NonParamState() = default;

    NonParamState (const std::initializer_list<StateValueBase*>& stateValues)
        : values (stateValues)
    {
    }

    NonParamState (NonParamState&&) noexcept = default;
    NonParamState& operator= (NonParamState&&) noexcept = default;

    /** Custom serializer */
    template <typename Serializer>
    static typename Serializer::SerializedType serialize (const NonParamState& state)
    {
        auto serial = Serializer::createBaseElement();
        for (const auto& value : state.values)
            value->serialize (serial);
        return serial;
    }

    /** Custom deserializer */
    template <typename Serializer>
    static void deserialize (typename Serializer::DeserializedType deserial, const NonParamState& state)
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

private:
    std::vector<StateValueBase*> values;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NonParamState)
};
} // namespace chowdsp
