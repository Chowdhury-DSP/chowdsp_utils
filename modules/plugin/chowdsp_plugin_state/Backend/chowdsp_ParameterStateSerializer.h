#pragma once

namespace chowdsp
{
/** Serialization/De-serialization functions for parameter state objects */
namespace ParameterStateSerializer
{
#ifndef DOXYGEN
    namespace detail
    {
        template <typename T, typename = void>
        struct ParameterElementTypeImpl;

        template <typename T>
        struct ParameterElementTypeImpl<T, typename std::enable_if<std::is_base_of_v<juce::AudioParameterFloat, T>>::type>
            : std::true_type
        {
            using base_type = juce::AudioParameterFloat;
            using element_type = float;
        };

        template <typename T>
        struct ParameterElementTypeImpl<T, typename std::enable_if<std::is_base_of_v<juce::AudioParameterChoice, T>>::type>
            : std::true_type
        {
            using base_type = juce::AudioParameterChoice;
            using element_type = int;
        };

        template <typename T>
        struct ParameterElementTypeImpl<T, typename std::enable_if<std::is_base_of_v<juce::AudioParameterBool, T>>::type>
            : std::true_type
        {
            using base_type = juce::AudioParameterBool;
            using element_type = bool;
        };

        /** Returns the base type of the parameter */
        template <typename ParamType>
        using ParameterBaseType = typename ParameterElementTypeImpl<ParamType>::base_type;

        /** Returns the element type of the parameter */
        template <typename ParamType>
        using ParameterElementType = typename ParameterElementTypeImpl<ParamType>::element_type;

        template <typename ParamType>
        ParameterElementType<ParamType> getSerializableValue (const ParamType& param)
        {
            if constexpr (std::is_base_of_v<juce::AudioParameterFloat, ParamType> || std::is_base_of_v<juce::AudioParameterBool, ParamType>)
                return param.get();
            else if constexpr (std::is_base_of_v<juce::AudioParameterChoice, ParamType>)
                return param.getIndex();
        }

        template <typename Serializer, typename ParamType>
        void deserializeParameter (const typename Serializer::SerializedType& serial, ParamType& param)
        {
            ParameterElementType<ParamType> val;
            Serialization::deserialize<Serializer> (serial, val);
            static_cast<ParameterBaseType<ParamType>&> (param) = val;
        }
    } // namespace detail
#endif

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
                                     Serializer::addChildElement (paramsSerial, detail::getSerializableValue (*paramHolder));
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
                                                 detail::deserializeParameter<Serializer> (serial, *paramHolder);
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
                                         paramHolder->setValueNotifyingHost (paramHolder->convertTo0to1 (static_cast<juce::AudioProcessorParameter&> (*paramHolder).getDefaultValue()));
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
