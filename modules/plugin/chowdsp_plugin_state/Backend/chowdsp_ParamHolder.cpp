namespace chowdsp
{
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wswitch-enum")
inline ParamHolder::ParamHolder (const juce::String& phName, bool phIsOwning)
    : name (phName),
      isOwning (phIsOwning)
{
    juce::ignoreUnused (isOwning);
}

inline ParamHolder::~ParamHolder()
{
    if (isOwning)
    {
        for (auto& thing : things)
        {
            if (getShouldDelete (thing))
            {
                switch (getType (thing))
                {
                    case FloatParam:
                        delete reinterpret_cast<FloatParameter*> (thing.get_ptr());
                        break;
                    case ChoiceParam:
                        delete reinterpret_cast<ChoiceParameter*> (thing.get_ptr());
                        break;
                    case BoolParam:
                        delete reinterpret_cast<BoolParameter*> (thing.get_ptr());
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

template <typename ParamType, typename... OtherParams>
std::enable_if_t<std::is_base_of_v<FloatParameter, ParamType>, void>
    ParamHolder::add (OptionalPointer<ParamType>& floatParam, OtherParams&... others)
{
    const auto paramID = toStringView (floatParam->paramID);
    ThingPtr paramPtr { reinterpret_cast<PackedVoid*> (isOwning ? floatParam.release() : floatParam.get()),
                        getFlags (FloatParam, isOwning) };
    allParamsMap.insert ({ paramID, paramPtr });
    things.insert (std::move (paramPtr));
    add (others...);
}

template <typename ParamType, typename... OtherParams>
std::enable_if_t<std::is_base_of_v<ChoiceParameter, ParamType>, void>
    ParamHolder::add (OptionalPointer<ParamType>& choiceParam, OtherParams&... others)
{
    const auto paramID = toStringView (choiceParam->paramID);
    ThingPtr paramPtr { reinterpret_cast<PackedVoid*> (isOwning ? choiceParam.release() : choiceParam.get()),
                              getFlags (ChoiceParam, isOwning) };
    allParamsMap.insert ({ paramID, paramPtr });
    things.insert (std::move (paramPtr));
    add (others...);
}

template <typename ParamType, typename... OtherParams>
std::enable_if_t<std::is_base_of_v<BoolParameter, ParamType>, void>
    ParamHolder::add (OptionalPointer<ParamType>& boolParam, OtherParams&... others)
{
    const auto paramID = toStringView (boolParam->paramID);
    ThingPtr paramPtr { reinterpret_cast<PackedVoid*> (isOwning ? boolParam.release() : boolParam.get()),
                              getFlags (BoolParam, isOwning) };
    allParamsMap.insert ({ paramID, paramPtr });
    things.insert (std::move (paramPtr));
    add (others...);
}

template <typename ParamType, typename... OtherParams>
std::enable_if_t<std::is_base_of_v<FloatParameter, ParamType>, void>
    ParamHolder::add (const OptionalPointer<ParamType>& floatParam, OtherParams&... others)
{
    jassert (! isOwning);
    add (const_cast<OptionalPointer<ParamType>&> (floatParam), others...);
}

template <typename ParamType, typename... OtherParams>
std::enable_if_t<std::is_base_of_v<ChoiceParameter, ParamType>, void>
    ParamHolder::add (const OptionalPointer<ParamType>& choiceParam, OtherParams&... others)
{
    jassert (! isOwning);
    add (const_cast<OptionalPointer<ParamType>&> (choiceParam), others...);
}

template <typename ParamType, typename... OtherParams>
std::enable_if_t<std::is_base_of_v<BoolParameter, ParamType>, void>
    ParamHolder::add (const OptionalPointer<ParamType>& boolParam, OtherParams&... others)
{
    jassert (! isOwning);
    add (const_cast<OptionalPointer<ParamType>&> (boolParam), others...);
}

template <typename... OtherParams>
void ParamHolder::add (ParamHolder& paramHolder, OtherParams&... others)
{
    allParamsMap.merge (paramHolder.allParamsMap);
    jassert (paramHolder.allParamsMap.empty()); // assuming no duplicate parameter IDs, all the parameters should be moved in the merge!
    things.insert (ThingPtr { reinterpret_cast<PackedVoid*> (&paramHolder), Holder });
    add (others...);
}

template <typename ParamContainerType, typename... OtherParams>
std::enable_if_t<TypeTraits::IsIterable<ParamContainerType>, void>
    ParamHolder::add (ParamContainerType& paramContainer, OtherParams&... others)
{
    for (auto& param : paramContainer)
        add (param);
    add (others...);
}

[[nodiscard]] inline int ParamHolder::count() const noexcept
{
    int count = static_cast<int> (things.count());
    for (auto& thing : things)
    {
        if (thing.get_flags() == Holder)
            count += reinterpret_cast<const ParamHolder*> (thing.get_ptr())->count() - 1;
    }
    return count;
}

inline void ParamHolder::clear()
{
    // It's generally not safe to clear the parameters if this is an owning ParamHolder
    // since we're almost certainly leaving some dangling references lying around!
    jassert (! isOwning);

    allParamsMap.clear();
    things.clear();
    arena.clear();
}

inline void ParamHolder::connectParametersToProcessor (juce::AudioProcessor& processor)
{
    for (auto& thing : things)
    {
        const auto type = getType (thing);
        if (type == Holder)
        {
            reinterpret_cast<ParamHolder*> (thing.get_ptr())->connectParametersToProcessor (processor);
            continue;
        }

        // Parameter must be non-null and owned by its pointer before being released to the processor
        jassert (thing.get_ptr() != nullptr && getShouldDelete (thing));
        switch (type)
        {
            case FloatParam:
                processor.addParameter (reinterpret_cast<FloatParameter*> (thing.get_ptr()));
                break;
            case ChoiceParam:
                processor.addParameter (reinterpret_cast<ChoiceParameter*> (thing.get_ptr()));
                break;
            case BoolParam:
                processor.addParameter (reinterpret_cast<BoolParameter*> (thing.get_ptr()));
                break;
            default:
                break;
        }
        thing.set_flags (getFlags (type, false));
    }
}

template <typename ParamCallable, typename ParamHolderCallable>
void ParamHolder::doForAllParametersOrContainers (ParamCallable&& paramCallable, ParamHolderCallable&& paramHolderCallable)
{
    for (auto& thing : things)
    {
        const auto type = getType (thing);
        if (type == Holder)
        {
            paramHolderCallable (*reinterpret_cast<ParamHolder*> (thing.get_ptr()));
            continue;
        }

        switch (type)
        {
            case FloatParam:
                paramCallable (*reinterpret_cast<FloatParameter*> (thing.get_ptr()));
                break;
            case ChoiceParam:
                paramCallable (*reinterpret_cast<ChoiceParameter*> (thing.get_ptr()));
                break;
            case BoolParam:
                paramCallable (*reinterpret_cast<BoolParameter*> (thing.get_ptr()));
                break;
            default:
                break;
        }
    }
}

template <typename ParamCallable, typename ParamHolderCallable>
void ParamHolder::doForAllParametersOrContainers (ParamCallable&& paramCallable, ParamHolderCallable&& paramHolderCallable) const
{
    for (auto& thing : things)
    {
        const auto type = getType (thing);
        if (type == Holder)
        {
            paramHolderCallable (*reinterpret_cast<const ParamHolder*> (thing.get_ptr()));
            continue;
        }

        switch (type)
        {
            case FloatParam:
                paramCallable (*reinterpret_cast<const FloatParameter*> (thing.get_ptr()));
                break;
            case ChoiceParam:
                paramCallable (*reinterpret_cast<const ChoiceParameter*> (thing.get_ptr()));
                break;
            case BoolParam:
                paramCallable (*reinterpret_cast<const BoolParameter*> (thing.get_ptr()));
                break;
            default:
                break;
        }
    }
}

template <typename Callable>
size_t ParamHolder::doForAllParameters (Callable&& callable, size_t index)
{
    doForAllParametersOrContainers (
        [&index, &callable] (auto& param)
        {
            callable (param, index++);
        },
        [&index, &callable] (ParamHolder& paramHolder)
        {
            index = paramHolder.doForAllParameters (std::forward<Callable> (callable), index);
        });
    return index;
}

template <typename Callable>
size_t ParamHolder::doForAllParameters (Callable&& callable, size_t index) const
{
    doForAllParametersOrContainers (
        [&index, &callable] (const auto& param)
        {
            callable (param, index++);
        },
        [&index, &callable] (const ParamHolder& paramHolder)
        {
            index = paramHolder.doForAllParameters (std::forward<Callable> (callable), index);
        });
    return index;
}

template <typename Serializer>
typename Serializer::SerializedType ParamHolder::serialize (const ParamHolder& paramHolder)
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
void ParamHolder::deserialize (typename Serializer::DeserializedType deserial, ParamHolder& paramHolder)
{
    std::vector<std::string_view> paramIDsThatHaveBeenDeserialized {};
    if (const auto numParamIDsAndVals = Serializer::getNumChildElements (deserial); numParamIDsAndVals % 2 == 0)
    {
        paramIDsThatHaveBeenDeserialized.reserve (static_cast<size_t> (numParamIDsAndVals) / 2);
        for (int i = 0; i < numParamIDsAndVals; i += 2)
        {
            const auto paramID = Serializer::getChildElement (deserial, i).template get<std::string_view>();
            const auto& paramDeserial = Serializer::getChildElement (deserial, i + 1);

            auto paramPtrIter = paramHolder.allParamsMap.find (std::string { paramID });
            if (paramPtrIter == paramHolder.allParamsMap.end())
                continue;

            paramIDsThatHaveBeenDeserialized.push_back (paramID);
            [&paramDeserial] (ThingPtr& paramPtr)
            {
                const auto deserializeParam = [] (auto* param, auto& pd)
                {
                    ParameterTypeHelpers::deserializeParameter<Serializer> (pd, *param);
                };

                const auto type = getType (paramPtr);
                switch (type)
                {
                    case FloatParam:
                        deserializeParam (reinterpret_cast<FloatParameter*> (paramPtr.get_ptr()), paramDeserial);
                    break;
                    case ChoiceParam:
                        deserializeParam (reinterpret_cast<ChoiceParameter*> (paramPtr.get_ptr()), paramDeserial);
                    break;
                    case BoolParam:
                        deserializeParam (reinterpret_cast<BoolParameter*> (paramPtr.get_ptr()), paramDeserial);
                    break;
                    default:
                        jassertfalse;
                        break;
                }
            }(paramPtrIter->second);
        }
    }
    else
    {
        jassertfalse; // state loading error
    }

    // set all un-matched objects to their default values
    if (! paramIDsThatHaveBeenDeserialized.empty())
    {
        paramHolder.doForAllParameters (
            [&paramIDsThatHaveBeenDeserialized] (auto& param, size_t)
            {
                if (std::find (paramIDsThatHaveBeenDeserialized.begin(),
                               paramIDsThatHaveBeenDeserialized.end(),
                               std::string_view { param.paramID.toRawUTF8(), param.paramID.getNumBytesAsUTF8() })
                    == paramIDsThatHaveBeenDeserialized.end())
                    ParameterTypeHelpers::resetParameter (param);
            });
    }
}

inline void ParamHolder::applyVersionStreaming (const Version& version)
{
    if (versionStreamingCallback != nullptr)
        versionStreamingCallback (version);

    for (auto& thing : things)
    {
        const auto type = getType (thing);
        if (type == Holder)
            reinterpret_cast<ParamHolder*> (thing.get_ptr())->applyVersionStreaming (version);
    }
}
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
} // namespace chowdsp
