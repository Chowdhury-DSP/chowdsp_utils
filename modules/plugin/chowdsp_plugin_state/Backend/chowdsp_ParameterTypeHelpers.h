#pragma once

namespace chowdsp
{
#ifndef DOXYGEN
/** This API is unstable and should not be used directly! */
namespace ParameterTypeHelpers
{
    template <typename T, typename = void>
    struct ParameterElementTypeImpl;

    template <typename T>
    struct ParameterElementTypeImpl<T, typename std::enable_if_t<std::is_base_of_v<juce::AudioParameterFloat, T>>>
        : std::true_type
    {
        using base_type = juce::AudioParameterFloat;
        using element_type = float;
    };

    template <typename T>
    struct ParameterElementTypeImpl<T, typename std::enable_if_t<std::is_base_of_v<juce::AudioParameterChoice, T>>>
        : std::true_type
    {
        using base_type = juce::AudioParameterChoice;
        using element_type = int;
    };

    template <typename T>
    struct ParameterElementTypeImpl<T, typename std::enable_if_t<std::is_base_of_v<juce::AudioParameterBool, T>>>
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
    ParameterElementType<ParamType> getValue (const ParamType& param)
    {
        if constexpr (std::is_base_of_v<juce::AudioParameterFloat, ParamType> || std::is_base_of_v<juce::AudioParameterBool, ParamType>)
            return param.get();
        else if constexpr (std::is_base_of_v<juce::AudioParameterChoice, ParamType>)
            return param.getIndex();
    }

    /** Resets a parameter to it's default value. */
    inline void resetParameter (juce::AudioProcessorParameter& param)
    {
        param.setValueNotifyingHost (param.getDefaultValue());
    }

    template <typename Serializer, typename ParamType>
    void serializeParameter (typename Serializer::SerializedType& serial, const ParamType& param)
    {
        Serializer::addChildElement (serial, param.paramID);
        Serializer::addChildElement (serial, getValue (param));
    }

    template <typename ParamType>
    void setValue (ParameterElementType<ParamType> val, ParamType& param)
    {
        static_cast<ParameterBaseType<ParamType>&> (param) = val;
    }

    template <typename Serializer, typename ParamType>
    void deserializeParameter (const typename Serializer::SerializedType& serial, ParamType& param)
    {
        ParameterElementType<ParamType> val;
        Serialization::deserialize<Serializer> (serial, val);
        setValue (val, param);
    }
} // namespace ParameterTypeHelpers
#endif
} // namespace chowdsp
