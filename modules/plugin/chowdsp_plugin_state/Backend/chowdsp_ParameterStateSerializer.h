#pragma once

namespace chowdsp
{
/** Serialization/De-serialization functions for parameter state objects */
namespace ParameterStateSerializer
{

    /** Helper method for serializing a plugin state object */
    template <typename Serializer, typename ParamsState>
    static typename Serializer::SerializedType serialize (const ParamsState& params)
    {
        auto paramsSerial = Serializer::createBaseElement();

        pfr::for_each_field (params,
                             [&paramsSerial] (const auto& paramHolder)
                             {
                                 using Type = std::decay_t<decltype (paramHolder)>;
                                 if constexpr (IsSmartPointer<Type>)
                                 {
                                     Serializer::addChildElement (paramsSerial, paramHolder->paramID);
                                     Serializer::addChildElement (paramsSerial, ParameterTypeHelpers::getSerializableValue (*paramHolder));
                                 }
                             });

        pfr::for_each_field (params,
                             [&paramsSerial] (const auto& paramHolder)
                             {
                                 using Type = std::decay_t<decltype (paramHolder)>;
                                 if constexpr (! IsSmartPointer<Type>)
                                 {
                                     Serializer::addChildElement (paramsSerial, Type::name);
                                     Serializer::addChildElement (paramsSerial, serialize<Serializer> (paramHolder));
                                 }
                             });

        return paramsSerial;
    }

    /** Helper method for deserializing a plugin state object */
    template <typename Serializer, typename ParamsState>
    static void deserialize (typename Serializer::DeserializedType paramsSerial, ParamsState& params)
    {
        juce::StringArray namesThatHaveBeenDeserialized {};
        if (const auto numParamIDsAndVals = Serializer::getNumChildElements (paramsSerial); numParamIDsAndVals % 2 == 0)
        {
            for (int i = 0; i < numParamIDsAndVals; i += 2)
            {
                juce::String name {};
                Serialization::deserialize<Serializer> (Serializer::getChildElement (paramsSerial, i), name);

                pfr::for_each_field (params,
                                     [i, &paramsSerial, &name, &namesThatHaveBeenDeserialized] (auto& paramHolder)
                                     {
                                         const auto serial = Serializer::getChildElement (paramsSerial, i + 1);

                                         using Type = std::decay_t<decltype (paramHolder)>;
                                         if constexpr (IsSmartPointer<Type>)
                                         {
                                             if (paramHolder->paramID == name)
                                                 ParameterTypeHelpers::deserializeParameter<Serializer> (serial, *paramHolder);
                                         }
                                         else
                                         {
                                             if (juce::String { Type::name.data() } == name)
                                                 deserialize<Serializer> (serial, paramHolder);
                                         }

                                         namesThatHaveBeenDeserialized.add (name);
                                     });

                jassert (namesThatHaveBeenDeserialized.contains (name)); // trying to load unknown parameter ID!
            }
        }
        else
        {
            jassertfalse; // state loading error
        }

        // set all un-matched parameters to their default values
        pfr::for_each_field (params,
                             [&namesThatHaveBeenDeserialized] (auto& paramHolder)
                             {
                                 using Type = std::decay_t<decltype (paramHolder)>;
                                 if constexpr (IsSmartPointer<Type>)
                                 {
                                     if (! namesThatHaveBeenDeserialized.contains (paramHolder->paramID))
                                         paramHolder->setValueNotifyingHost (static_cast<juce::AudioProcessorParameter&> (*paramHolder).getDefaultValue());
                                 }
                                 else
                                 {
                                     if (! namesThatHaveBeenDeserialized.contains (Type::name.data()))
                                         deserialize<Serializer> ({}, paramHolder);
                                 }
                             });
    }
} // namespace ParameterStateSerializer
} // namespace chowdsp
