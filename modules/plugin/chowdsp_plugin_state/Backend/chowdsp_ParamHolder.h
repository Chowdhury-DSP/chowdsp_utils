#pragma once

namespace chowdsp
{
//namespace param_holder_detail
//{
//    template <typename ParamHolder>
//    struct Iterator
//    {
//        using iterator_category = std::forward_iterator_tag;
//        using value_type        = juce::RangedAudioParameter;
//        using pointer           = value_type*;
//        using reference         = value_type&;
//
//        explicit Iterator (ParamHolder& paramHolder) : pHolder (paramHolder) {}
//
//        reference operator*() const { return *ptr; }
//        pointer operator->() { return ptr; }
//
//        // Prefix increment
//        Iterator& operator++()
//        {
//            ptr++;
//
//            if (ptr == pHolder.floatParams.end())
//                ptr = pHolder.choiceParams.begin();
//
//            if (ptr == pHolder.choiceParams.end())
//                ptr = pHolder.boolParams.begin();
//
//            if (ptr == pHolder.boolParams.end())
//            {
//
//            }
//
//            ptr++; return *this;
//        }
//
//        friend bool operator== (const Iterator& a, const Iterator& b) { return a.ptr == b.ptr; };
//        friend bool operator!= (const Iterator& a, const Iterator& b) { return a.ptr != b.ptr; };
//
//    private:
//        ParamHolder& pHolder;
//        pointer ptr;
//    };
//}

class ParamHolder
{
public:
    ParamHolder() = default;

    template <typename ParamType, typename... OtherParams>
    std::enable_if_t<std::is_base_of_v<FloatParameter, ParamType>, void>
        add (OptionalPointer<ParamType>& floatParam, OtherParams&... others)
    {
        OptionalPointer<FloatParameter> paramPtr {};
        paramPtr.setOwning (floatParam.release());
        floatParams.push_back (std::move (paramPtr));
        add (others...);
    }

    template <typename ParamType, typename... OtherParams>
    std::enable_if_t<std::is_base_of_v<ChoiceParameter, ParamType>, void>
        add (OptionalPointer<ParamType>& choiceParam, OtherParams&... others)
    {
        OptionalPointer<ChoiceParameter> paramPtr {};
        paramPtr.setOwning (choiceParam.release());
        choiceParams.push_back (std::move (paramPtr));
        add (others...);
    }

    template <typename ParamType, typename... OtherParams>
    std::enable_if_t<std::is_base_of_v<BoolParameter, ParamType>, void>
        add (OptionalPointer<ParamType>& boolParam, OtherParams&... others)
    {
        OptionalPointer<BoolParameter> paramPtr {};
        paramPtr.setOwning (boolParam.release());
        boolParams.push_back (std::move (paramPtr));
        add (others...);
    }

    template <typename ParamType, typename... OtherParams>
    std::enable_if_t<std::is_base_of_v<ParamHolder, ParamType>, void>
        add (ParamType& paramHolder, OtherParams&... others)
    {
        otherParams.push_back (&paramHolder);
        add (others...);
    }

    [[nodiscard]] int count() const noexcept
    {
        int numParams = int (floatParams.size() + choiceParams.size() + boolParams.size());
        for (const auto& holder : otherParams)
            numParams += holder->count();
        return numParams;
    }

    /** Internal use only! */
    template <typename ParamContainersCallable, typename ParamHolderCallable>
    void doForAllParameterContainers (ParamContainersCallable&& paramContainersCallable, ParamHolderCallable&& paramHolderCallable)
    {
        paramContainersCallable (floatParams);
        paramContainersCallable (choiceParams);
        paramContainersCallable (boolParams);
        for (auto& holder : otherParams)
            paramHolderCallable (*holder);
    }

    /** Internal use only! */
    template <typename ParamContainersCallable, typename ParamHolderCallable>
    void doForAllParameterContainers (ParamContainersCallable&& paramContainersCallable, ParamHolderCallable&& paramHolderCallable) const
    {
        paramContainersCallable (floatParams);
        paramContainersCallable (choiceParams);
        paramContainersCallable (boolParams);
        for (const auto& holder : otherParams)
            paramHolderCallable (*holder);
    }

    void connectParametersToProcessor (juce::AudioProcessor& processor)
    {
        doForAllParameterContainers (
            [&processor] (auto& paramVec)
            {
                for (auto& param : paramVec)
                {
                    // Parameter must be non-null and owned by its pointer before being released to the processor
                    jassert (param != nullptr && param.isOwner());
                    processor.addParameter (param.release());
                }
            },
            [&processor] (auto& holder)
            {
                holder.connectParametersToProcessor (processor);
            });
    }

    template <typename Callable>
    size_t doForAllParameters (Callable&& callable, size_t index = 0)
    {
        doForAllParameterContainers (
            [&index, call = std::forward<Callable> (callable)] (auto& paramVec)
            {
                for (auto& param : paramVec)
                    call (*param, index++);
            },
            [&index, call = std::forward<Callable> (callable)] (auto& holder)
            {
                index = holder.doForAllParameters (std::move (call), index);
            });

        return index;
    }

    template <typename Callable>
    size_t doForAllParameters (Callable&& callable, size_t index = 0) const
    {
        doForAllParameterContainers (
            [&index, call = std::forward<Callable> (callable)] (const auto& paramVec)
            {
                for (const auto& param : paramVec)
                    call (*param, index++);
            },
            [&index, call = std::forward<Callable> (callable)] (const auto& holder)
            {
                index = holder.doForAllParameters (std::move (call), index);
            });

        return index;
    }

    template <typename Serializer>
    static typename Serializer::SerializedType serialize (const ParamHolder& paramHolder)
    {
        auto serial = Serializer::createBaseElement();
        paramHolder.doForAllParameters (
            [&serial] (auto& param, size_t)
            {
                ParameterTypeHelpers::serializeParameter<Serializer> (serial, param);
            });
        return serial;
    }

    template <typename Serializer>
    static void deserialize (typename Serializer::DeserializedType deserial, ParamHolder& paramHolder)
    {
        juce::StringArray paramIDsThatHaveBeenDeserialized {};
        if (const auto numParamIDsAndVals = Serializer::getNumChildElements (deserial); numParamIDsAndVals % 2 == 0)
        {
            for (int i = 0; i < numParamIDsAndVals; i += 2)
            {
                juce::String paramID {};
                Serialization::deserialize<Serializer> (Serializer::getChildElement (deserial, i), paramID);
                paramHolder.doForAllParameters (
                    [&deserial, &paramID = std::as_const (paramID), &paramIDsThatHaveBeenDeserialized] (auto& param, size_t)
                    {
                        if (param.paramID == paramID)
                        {
                            ParameterTypeHelpers::deserializeParameter<Serializer> (deserial, param);
                            paramIDsThatHaveBeenDeserialized.add (paramID);
                        }
                    });
            }
        }
        else
        {
            jassertfalse; // state loading error
        }

        // set all un-matched objects to their default values
        paramHolder.doForAllParameters (
            [&paramIDsThatHaveBeenDeserialized] (auto& param, size_t)
            {
                if (! paramIDsThatHaveBeenDeserialized.contains (param.paramID))
                    param.setValueNotifyingHost (static_cast<juce::AudioProcessorParameter&> (param).getDefaultValue());
            });
    }

private:
    void add()
    {
        // base case!
    }

    std::vector<OptionalPointer<FloatParameter>> floatParams;
    std::vector<OptionalPointer<ChoiceParameter>> choiceParams;
    std::vector<OptionalPointer<BoolParameter>> boolParams;
    std::vector<ParamHolder*> otherParams;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParamHolder)
};
} // namespace chowdsp
