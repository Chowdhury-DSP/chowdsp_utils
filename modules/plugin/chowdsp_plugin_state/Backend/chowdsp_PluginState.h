#pragma once

namespace chowdsp
{
#ifndef DOXYGEN
namespace detail
{
    template <typename ParamsStateType>
    void addParamsToProcessor (juce::AudioProcessor& processor, ParamsStateType& paramsState)
    {
        pfr::for_each_field (paramsState,
                             [&processor] (auto& paramHolder)
                             {
                                 using Type = std::decay_t<decltype (paramHolder)>;
                                 if constexpr (IsSmartPointer<Type>)
                                 {
                                     using ParamType = typename std::decay_t<decltype (paramHolder)>::element_type;
                                     static_assert (std::is_base_of_v<juce::RangedAudioParameter, ParamType>,
                                                    "All parameters must be SmartPointers of JUCE parameter types!");

                                     processor.addParameter (paramHolder.release());
                                 }
                                 else
                                 {
                                     addParamsToProcessor (processor, paramHolder); // add nested params!
                                 }
                             });
    }
} // namespace detail
#endif

struct NullState
{
};

template <typename ParameterState, typename NonParameterState = NullState, typename Serializer = JSONSerializer>
class PluginState
{
public:
    explicit PluginState (juce::AudioProcessor& processor)
    {
        detail::addParamsToProcessor (processor, params);
    }

    void serialize (juce::MemoryBlock& data)
    {
        Serialization::serialize<Serializer> (*this, data);
    }

    void deserialize (const juce::MemoryBlock& data)
    {
        Serialization::deserialize<Serializer> (data, *this);
    }

    template <typename S>
    static typename S::SerializedType serialize (const PluginState& object)
    {
        auto serial = S::createBaseElement();
        S::addChildElement (serial, ParameterStateSerializer::serialize<S> (object.params));
        S::addChildElement (serial, Serialization::serialize<S> (object.nonParams));
        return serial;
    }

    template <typename S>
    static void deserialize (typename S::DeserializedType serial, PluginState& object)
    {
        if (S::getNumChildElements (serial) != 2)
        {
            jassertfalse; // state load error!
            return;
        }

        ParameterStateSerializer::deserialize<S> (S::getChildElement (serial, 0), object.params);
        Serialization::deserialize<S> (S::getChildElement (serial, 1), object.nonParams);
    }

    ParameterState params;
    NonParameterState nonParams;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginState)
};
} // namespace chowdsp
