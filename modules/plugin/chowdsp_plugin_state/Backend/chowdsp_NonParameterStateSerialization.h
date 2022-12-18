#pragma once

namespace chowdsp
{
#ifndef DOXYGEN
/** Helpers for serializing non-parameter state. */
namespace NonParameterStateSerialization
{
    namespace detail
    {
        template <class T>
        struct IsStateValueType : std::false_type
        {
        };

        template <class T>
        struct IsStateValueType<StateValue<T>> : std::true_type
        {
        };

        template <typename T>
        static constexpr bool IsStateValue = IsStateValueType<T>::value;
    } // namespace detail

    /** Helper method for serializing a non-parameter state object */
    template <typename Serializer, typename StateObjectType>
    static typename Serializer::SerializedType serialize (const StateObjectType& state)
    {
        auto serial = Serializer::createBaseElement();

        pfr::for_each_field (state,
                             [&serial] (const auto& stateObject)
                             {
                                 using Type = std::decay_t<decltype (stateObject)>;
                                 static_assert (detail::IsStateValue<Type>, "Non-parameter state must consist only of StateValue objects!");
                                 Type::template serialize<Serializer> (serial, stateObject);
                             });
        return serial;
    }

    /** Helper method for deserializing a non-parameter state object */
    template <typename Serializer, typename StateObjectType>
    static void deserialize (typename Serializer::DeserializedType serial, StateObjectType& state)
    {
        juce::StringArray namesThatHaveBeenDeserialized {};
        if (const auto numNamesAndVals = Serializer::getNumChildElements (serial); numNamesAndVals % 2 == 0)
        {
            for (int i = 0; i < numNamesAndVals; i += 2)
            {
                juce::String name {};
                Serialization::deserialize<Serializer> (Serializer::getChildElement (serial, i), name);

                pfr::for_each_field (state,
                                     [i, &serial, &name, &namesThatHaveBeenDeserialized] (auto& stateObject)
                                     {
                                         const auto elementSerial = Serializer::getChildElement (serial, i + 1);

                                         using Type = std::decay_t<decltype (stateObject)>;
                                         if (toString (stateObject.name) == name)
                                         {
                                             Type::template deserialize<Serializer> (elementSerial, stateObject);
                                             namesThatHaveBeenDeserialized.add (name);
                                         }
                                     });

                jassert (namesThatHaveBeenDeserialized.contains (name)); // trying to load unknown parameter ID!
            }
        }
        else
        {
            jassertfalse; // state loading error
        }

        // set all un-matched objects to their default values
        pfr::for_each_field (state,
                             [&namesThatHaveBeenDeserialized] (auto& stateObject)
                             {
                                 if (! namesThatHaveBeenDeserialized.contains (toString (stateObject.name)))
                                     stateObject.reset();
                             });
    }
} // namespace NonParameterStateSerialization
#endif
} // namespace chowdsp
