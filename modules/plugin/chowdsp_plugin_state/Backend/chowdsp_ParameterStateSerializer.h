#pragma once

namespace chowdsp
{
/** Serialization/De-serialization functions for parameter or non-parameter state objects */
namespace PluginStateSerializer
{
#ifndef DOXYGEN
    namespace detail
    {
        template <typename T>
        constexpr std::string_view getObjectName (const T& object)
        {
            std::string_view name {};
            pfr::for_each_field (object,
                                 [&name] (const auto& field)
                                 {
                                     using Type = std::decay_t<decltype (field)>;
                                     if constexpr (ParameterTypeHelpers::IsStringType<Type>)
                                         if (name.empty())
                                             name = field;
                                 });

            if (! name.empty())
                return name;

            return nameof::nameof_type<T>();
        }
    } // namespace detail
#endif

    /** Helper method for serializing a plugin state object */
    template <typename Serializer, typename StateType>
    static typename Serializer::SerializedType serialize (const StateType& state)
    {
        auto serial = Serializer::createBaseElement();

        pfr::for_each_field (state,
                             [&serial] (const auto& stateObject)
                             {
                                 using Type = std::decay_t<decltype (stateObject)>;
                                 if constexpr (ParameterTypeHelpers::IsParameterPointerType<Type>)
                                 {
                                     ParameterTypeHelpers::serializeParameter<Serializer> (serial, *stateObject);
                                 }
                                 else if constexpr (PluginStateHelpers::IsStateValue<Type>)
                                 {
                                     Type::template serialize<Serializer> (serial, stateObject);
                                 }
                             });

        pfr::for_each_field (state,
                             [&serial] (const auto& stateObject)
                             {
                                 using Type = std::decay_t<decltype (stateObject)>;
                                 if constexpr (TypeTraits::IsIterable<Type> && ! ParameterTypeHelpers::IsStringType<Type>)
                                 {
                                     for (const auto& [index, elem] : enumerate (stateObject))
                                     {
                                         const auto elementTag = juce::String { detail::getObjectName (elem).data() } + "_" + juce::String (index);
                                         Serializer::addChildElement (serial, elementTag);
                                         Serializer::addChildElement (serial, serialize<Serializer> (elem));
                                     }
                                 }
                             });

        pfr::for_each_field (state,
                             [&serial] (const auto& stateObject)
                             {
                                 using Type = std::decay_t<decltype (stateObject)>;
                                 if constexpr (! (ParameterTypeHelpers::IsParameterPointerType<Type> || PluginStateHelpers::IsStateValue<Type> || ParameterTypeHelpers::IsHelperType<Type> || TypeTraits::IsIterable<Type>) )
                                 {
                                     Serializer::addChildElement (serial, detail::getObjectName (stateObject));
                                     Serializer::addChildElement (serial, serialize<Serializer> (stateObject));
                                 }
                             });

        return serial;
    }

    /** Helper method for deserializing a plugin state object */
    template <typename Serializer, typename StateType>
    static void deserialize (typename Serializer::DeserializedType serial, StateType& state)
    {
        juce::StringArray namesThatHaveBeenDeserialized {};
        if (const auto numParamIDsAndVals = Serializer::getNumChildElements (serial); numParamIDsAndVals % 2 == 0)
        {
            for (int i = 0; i < numParamIDsAndVals; i += 2)
            {
                juce::String name {};
                Serialization::deserialize<Serializer> (Serializer::getChildElement (serial, i), name);

                pfr::for_each_field (state,
                                     [i, &serial, &name, &namesThatHaveBeenDeserialized] (auto& stateObject)
                                     {
                                         const auto elementSerial = Serializer::getChildElement (serial, i + 1);

                                         using Type = std::decay_t<decltype (stateObject)>;
                                         if constexpr (ParameterTypeHelpers::IsParameterPointerType<Type>)
                                         {
                                             if (stateObject->paramID == name)
                                             {
                                                 ParameterTypeHelpers::deserializeParameter<Serializer> (elementSerial, *stateObject);
                                                 namesThatHaveBeenDeserialized.add (name);
                                             }
                                         }
                                         else if constexpr (PluginStateHelpers::IsStateValue<Type>)
                                         {
                                             if (toString (stateObject.name) == name)
                                             {
                                                 Type::template deserialize<Serializer> (elementSerial, stateObject);
                                                 namesThatHaveBeenDeserialized.add (name);
                                             }
                                         }
                                         else if constexpr (ParameterTypeHelpers::IsHelperType<Type>)
                                         {
                                             namesThatHaveBeenDeserialized.add (name);
                                         }
                                         else if constexpr (TypeTraits::IsIterable<Type> && ! ParameterTypeHelpers::IsStringType<Type>)
                                         {
                                             for (const auto& [index, elem] : enumerate (stateObject))
                                             {
                                                 const auto elementTag = juce::String { detail::getObjectName (elem).data() } + "_" + juce::String (index);
                                                 if (elementTag == name)
                                                 {
                                                     deserialize<Serializer> (elementSerial, elem);
                                                     namesThatHaveBeenDeserialized.add (name);
                                                 }
                                             }
                                         }
                                         else
                                         {
                                             if (toString (detail::getObjectName (stateObject)) == name)
                                             {
                                                 deserialize<Serializer> (elementSerial, stateObject);
                                                 namesThatHaveBeenDeserialized.add (name);
                                             }
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
                                 using Type = std::decay_t<decltype (stateObject)>;
                                 if constexpr (ParameterTypeHelpers::IsParameterPointerType<Type>)
                                 {
                                     if (! namesThatHaveBeenDeserialized.contains (stateObject->paramID))
                                         stateObject->setValueNotifyingHost (static_cast<juce::AudioProcessorParameter&> (*stateObject).getDefaultValue());
                                 }
                                 else if constexpr (PluginStateHelpers::IsStateValue<Type>)
                                 {
                                     if (! namesThatHaveBeenDeserialized.contains (toString (stateObject.name)))
                                         stateObject.reset();
                                 }
                                 else if constexpr (ParameterTypeHelpers::IsHelperType<Type>)
                                 {
                                     return; // nothing to do
                                 }
                                 else if constexpr (TypeTraits::IsIterable<Type> && ! ParameterTypeHelpers::IsStringType<Type>)
                                 {
                                     for (const auto& [index, elem] : enumerate (stateObject))
                                     {
                                         const auto elementTag = juce::String { detail::getObjectName (elem).data() } + "_" + juce::String (index);
                                         if (! namesThatHaveBeenDeserialized.contains (elementTag))
                                             deserialize<Serializer> ({}, elem);
                                     }
                                 }
                                 else
                                 {
                                     if (! namesThatHaveBeenDeserialized.contains (toString (detail::getObjectName (stateObject))))
                                         deserialize<Serializer> ({}, stateObject);
                                 }
                             });
    }
} // namespace PluginStateSerializer
} // namespace chowdsp
