#pragma once

namespace chowdsp
{

struct NullState
{
};

template <typename ParameterState, typename NonParameterState = NullState, typename Serializer = JSONSerializer>
class PluginState
{
public:
    explicit PluginState (juce::AudioProcessor& processor)
    {
        // TODO: what about parameter groups, or nested parameter trees in general?

        pfr::for_each_field (params,
                             [&processor] (auto& paramHolder)
                             {
                                 using ParamType = typename std::decay_t<decltype (paramHolder)>::element_type;
                                 static_assert (std::is_base_of_v<juce::RangedAudioParameter, ParamType>,
                                                "All parameters must be OptionallyOwnedPointers of JUCE parameter types!");

                                 processor.addParameter (paramHolder.release());
                             });
    }

    void serialize (juce::MemoryBlock& data)
    {
        const auto serial = Serialization::serialize<Serializer> (*this);
        JSONUtils::toMemoryBlock (serial, data);
    }

    void deserialize (const juce::MemoryBlock& data)
    {
        json deserial;
        try
        {
            deserial = JSONUtils::fromMemoryBlock (data);
        }
        catch (const std::exception& e)
        {
            jassertfalse;
            juce::Logger::writeToLog (juce::String { "Error reading saved state! " } + juce::String { e.what() });
            return;
        }

        Serialization::deserialize<Serializer> (deserial, *this);
    }

    template <typename S>
    static typename S::SerializedType serialize (const PluginState& object)
    {
        auto paramsSerial = S::createBaseElement();
        pfr::for_each_field (object.params,
                             [&paramsSerial] (const auto& paramHolder)
                             {
                                 S::addChildElement (paramsSerial, paramHolder->paramID);
                                 S::addChildElement (paramsSerial, paramHolder->get());
                             });

        auto fullSerial = S::createBaseElement();
        S::addChildElement (fullSerial, std::move (paramsSerial));
        S::addChildElement (fullSerial, Serialization::serialize<S> (object.nonParams));
        return fullSerial;
    }

    template <typename S>
    static void deserialize (typename S::DeserializedType serial, PluginState& object)
    {
        if (S::getNumChildElements (serial) != 2)
        {
            jassertfalse; // state load error!
            return;
        }

        const auto paramsSerial = S::getChildElement (serial, 0);
        juce::StringArray paramIDsThatHaveBeenDeserialized {};
        if (const auto numParamIDsAndVals = S::getNumChildElements (paramsSerial); numParamIDsAndVals % 2 == 0)
        {
            for (int i = 0; i < numParamIDsAndVals; i += 2)
            {
                juce::String paramID {};
                Serialization::deserialize<S> (S::getChildElement (paramsSerial, i), paramID);

                pfr::for_each_field (object.params,
                                     [i, &paramsSerial, &paramID, &paramIDsThatHaveBeenDeserialized] (auto& paramHolder)
                                     {
                                         if (paramHolder->paramID == paramID)
                                         {
                                             float val;
                                             Serialization::deserialize<S> (S::getChildElement (paramsSerial, i + 1), val);
                                             paramHolder->setValueNotifyingHost (paramHolder->convertTo0to1 (val));
                                             paramIDsThatHaveBeenDeserialized.add (paramID);
                                         }
                                     });

                jassert (paramIDsThatHaveBeenDeserialized.contains (paramID)); // trying to load unknown parameter ID!
            }
        }
        else
        {
            jassertfalse; // state loading error
        }

        // set all un-matched parameters to their default values
        pfr::for_each_field (object.params,
                             [&paramIDsThatHaveBeenDeserialized] (auto& paramHolder)
                             {
                                 if (! paramIDsThatHaveBeenDeserialized.contains (paramHolder->paramID))
                                     paramHolder->setValueNotifyingHost (paramHolder->convertTo0to1 (paramHolder->getDefaultValue()));
                             });

        Serialization::deserialize<S> (S::getChildElement (serial, 1), object.nonParams);
    }

    ParameterState params;
    NonParameterState nonParams;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginState)
};
} // namespace chowdsp
