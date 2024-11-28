namespace chowdsp
{
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wswitch-enum")
inline ParamHolder::ParamHolder (ParamHolder* parent, std::string_view phName, bool phIsOwning)
    : arena {
          [parent]
          {
              if (parent != nullptr)
              {
                  jassert (parent->arena != nullptr);
                  return OptionalPointer<ChainedArenaAllocator> { parent->arena.get(), false };
              }
              return OptionalPointer<ChainedArenaAllocator> { static_cast<size_t> (1024) };
          }(),
      },
#if CHOWDSP_USE_LEGACY_STATE_SERIALIZATION
      allParamsMap { MapAllocator { arena } },
#endif
      name { arena::alloc_string (*arena, phName) },
      isOwning { phIsOwning }
{
}

inline ParamHolder::ParamHolder (ChainedArenaAllocator& alloc, std::string_view phName, bool phIsOwning)
    : arena { &alloc, false },
#if CHOWDSP_USE_LEGACY_STATE_SERIALIZATION
      allParamsMap { MapAllocator { arena } },
#endif
      name { arena::alloc_string (*arena, phName) },
      isOwning { phIsOwning }
{
}

inline ParamHolder::~ParamHolder()
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

template <typename ParamType, typename... OtherParams>
std::enable_if_t<std::is_base_of_v<FloatParameter, ParamType>, void>
    ParamHolder::add (OptionalPointer<ParamType>& floatParam, OtherParams&... others)
{
    ThingPtr paramPtr { reinterpret_cast<PackedVoid*> (isOwning ? floatParam.release() : floatParam.get()),
                        getFlags (FloatParam, isOwning) };
#if CHOWDSP_USE_LEGACY_STATE_SERIALIZATION
    const auto paramID = toStringView (floatParam->paramID);
    allParamsMap.insert ({ paramID, paramPtr });
#endif
    things.insert (std::move (paramPtr));
    add (others...);
}

template <typename ParamType, typename... OtherParams>
std::enable_if_t<std::is_base_of_v<ChoiceParameter, ParamType>, void>
    ParamHolder::add (OptionalPointer<ParamType>& choiceParam, OtherParams&... others)
{
    ThingPtr paramPtr { reinterpret_cast<PackedVoid*> (isOwning ? choiceParam.release() : choiceParam.get()),
                        getFlags (ChoiceParam, isOwning) };
#if CHOWDSP_USE_LEGACY_STATE_SERIALIZATION
    const auto paramID = toStringView (choiceParam->paramID);
    allParamsMap.insert ({ paramID, paramPtr });
#endif
    things.insert (std::move (paramPtr));
    add (others...);
}

template <typename ParamType, typename... OtherParams>
std::enable_if_t<std::is_base_of_v<BoolParameter, ParamType>, void>
    ParamHolder::add (OptionalPointer<ParamType>& boolParam, OtherParams&... others)
{
    ThingPtr paramPtr { reinterpret_cast<PackedVoid*> (isOwning ? boolParam.release() : boolParam.get()),
                        getFlags (BoolParam, isOwning) };
#if CHOWDSP_USE_LEGACY_STATE_SERIALIZATION
    const auto paramID = toStringView (boolParam->paramID);
    allParamsMap.insert ({ paramID, paramPtr });
#endif
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
#if CHOWDSP_USE_LEGACY_STATE_SERIALIZATION
    // This should be the parent of the holder being added.
    jassert (arena == paramHolder.arena);

    allParamsMap.merge (paramHolder.allParamsMap);
    jassert (paramHolder.allParamsMap.empty()); // assuming no duplicate parameter IDs, all the parameters should be moved in the merge!
#endif
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
    auto count = static_cast<int> (things.count());
    for (auto& thing : things)
    {
        if (thing.get_flags() == Holder)
            count += reinterpret_cast<const ParamHolder*> (thing.get_ptr())->count() - 1;
    }
    return count;
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
#if ! CHOWDSP_USE_LEGACY_STATE_SERIALIZATION
    auto serial = nlohmann::json::object();
    paramHolder.doForAllParameters (
        [&serial] (auto& param, size_t)
        {
            const auto paramID = toStringView (param.paramID);
            serial[paramID] = ParameterTypeHelpers::getValue (param);
        });
    return serial;
#else
    auto serial = Serializer::createBaseElement();
    paramHolder.doForAllParameters (
        [&serial] (auto& param, size_t)
        {
            ParameterTypeHelpers::serializeParameter<Serializer> (serial, param);
        });
    return serial;
#endif
}

template <typename Serializer>
void ParamHolder::deserialize (typename Serializer::DeserializedType deserial, ParamHolder& paramHolder)
{
#if ! CHOWDSP_USE_LEGACY_STATE_SERIALIZATION
    paramHolder.doForAllParameters (
        [&deserial] (auto& param, size_t)
        {
            const auto paramID = toStringView (param.paramID);
            ParameterTypeHelpers::setValue (deserial.value (paramID, ParameterTypeHelpers::getDefaultValue (param)), param);
        });
#else
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
#endif
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
