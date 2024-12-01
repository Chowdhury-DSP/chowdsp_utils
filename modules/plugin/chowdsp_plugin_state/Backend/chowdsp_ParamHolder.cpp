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
              return OptionalPointer<ChainedArenaAllocator> { static_cast<size_t> (32 * CHOWDSP_PLUGIN_STATE_MAX_PARAM_COUNT) };
          }(),
      },
      name { arena::alloc_string (*arena, phName) },
      isOwning { phIsOwning }
{
}

inline ParamHolder::ParamHolder (ChainedArenaAllocator& alloc, std::string_view phName, bool phIsOwning)
    : arena { &alloc, false },
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

    if (arena.isOwner())
    {
        // If you're hitting this assertion, you probably want to increase
        // CHOWDSP_PLUGIN_STATE_MAX_PARAM_COUNT.
        jassert (arena->get_extra_alloc_list() == nullptr);
    }
}

template <typename ParamType, typename... OtherParams>
std::enable_if_t<std::is_base_of_v<FloatParameter, ParamType>, void>
    ParamHolder::add (OptionalPointer<ParamType>& floatParam, OtherParams&... others)
{
    things.insert (ThingPtr { reinterpret_cast<PackedVoid*> (isOwning ? floatParam.release() : floatParam.get()),
                              getFlags (FloatParam, isOwning) });
    add (others...);
}

template <typename ParamType, typename... OtherParams>
std::enable_if_t<std::is_base_of_v<ChoiceParameter, ParamType>, void>
    ParamHolder::add (OptionalPointer<ParamType>& choiceParam, OtherParams&... others)
{
    things.insert (ThingPtr { reinterpret_cast<PackedVoid*> (isOwning ? choiceParam.release() : choiceParam.get()),
                              getFlags (ChoiceParam, isOwning) });
    add (others...);
}

template <typename ParamType, typename... OtherParams>
std::enable_if_t<std::is_base_of_v<BoolParameter, ParamType>, void>
    ParamHolder::add (OptionalPointer<ParamType>& boolParam, OtherParams&... others)
{
    things.insert (ThingPtr { reinterpret_cast<PackedVoid*> (isOwning ? boolParam.release() : boolParam.get()),
                              getFlags (BoolParam, isOwning) });
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

inline void ParamHolder::getParameterPointers (ParamHolder& holder, ParamDeserialVec& parameters)
{
    for (auto& thing : holder.things)
    {
        const auto type = getType (thing);
        if (type == Holder)
        {
            getParameterPointers (*reinterpret_cast<ParamHolder*> (thing.get_ptr()), parameters);
            continue;
        }

        std::string_view paramID {};
        switch (type)
        {
            case FloatParam:
                paramID = toStringView (reinterpret_cast<FloatParameter*> (thing.get_ptr())->paramID);
                break;
            case ChoiceParam:
                paramID = toStringView (reinterpret_cast<ChoiceParameter*> (thing.get_ptr())->paramID);
                break;
            case BoolParam:
                paramID = toStringView (reinterpret_cast<BoolParameter*> (thing.get_ptr())->paramID);
                break;
            default:
                break;
        }

        parameters.emplace_back (paramID, thing);
    }
}

inline void ParamHolder::serialize (ChainedArenaAllocator& arena, const ParamHolder& paramHolder)
{
    auto* serialize_num_bytes = arena.allocate<bytes_detail::size_type> (1, 1);
    size_t num_bytes = 0;
    paramHolder.doForAllParameters (
        [&] (auto& param, size_t)
        {
            num_bytes += serialize_string (toStringView (param.paramID), arena);
            num_bytes += serialize_object (ParameterTypeHelpers::getValue (param), arena);
        });
    serialize_direct (serialize_num_bytes, num_bytes);
}

inline void ParamHolder::deserialize (nonstd::span<const std::byte>& serial_data, ParamHolder& paramHolder)
{
    using namespace ParameterTypeHelpers;
    auto num_bytes = deserialize_direct<bytes_detail::size_type> (serial_data);
    if (num_bytes == 0)
    {
        paramHolder.doForAllParameters (
            [&] (auto& param, size_t)
            {
                ParameterTypeHelpers::resetParameter (param);
            });
        return;
    }

    auto data = serial_data.subspan (0, num_bytes);
    serial_data = serial_data.subspan (num_bytes);

    const auto _ = paramHolder.arena->create_frame();
    ParamDeserialVec parameters { ParamDeserialAlloc { *paramHolder.arena } };
    parameters.reserve ((size_t) paramHolder.count());
    getParameterPointers (paramHolder, parameters);

    auto params_iter = parameters.begin();
    const auto get_param_ptr = [&] (std::string_view paramID) -> ThingPtr
    {
        for (auto iter = params_iter; iter != parameters.end(); ++iter)
        {
            if (iter->first == paramID)
            {
                auto ptr = iter->second;
                params_iter = parameters.erase (iter);
                return ptr;
            }
        }

        for (auto iter = parameters.begin(); iter != params_iter; ++iter)
        {
            if (iter->first == paramID)
            {
                auto ptr = iter->second;
                params_iter = parameters.erase (iter);
                return ptr;
            }
        }
        return {};
    };

    while (data.size() > 0)
    {
        const auto param_id = deserialize_string (data);
        auto param_ptr = get_param_ptr (param_id);
        if (param_ptr == nullptr)
        {
            const auto param_num_bytes = deserialize_direct<bytes_detail::size_type> (data);
            data = data.subspan (param_num_bytes);
            continue;
        }

        const auto type = getType (param_ptr);
        switch (type)
        {
            case FloatParam:
                setValue (deserialize_object<ParameterElementType<FloatParameter>> (data),
                          *reinterpret_cast<FloatParameter*> (param_ptr.get_ptr()));
                break;
            case ChoiceParam:
                setValue (deserialize_object<ParameterElementType<ChoiceParameter>> (data),
                          *reinterpret_cast<ChoiceParameter*> (param_ptr.get_ptr()));
                break;
            case BoolParam:
                setValue (deserialize_object<ParameterElementType<BoolParameter>> (data),
                          *reinterpret_cast<BoolParameter*> (param_ptr.get_ptr()));
                break;
            default:
                break;
        }
    }
    /*
    for (auto [param_id, param_ptr] : parameters)
    {
        const auto type = getType (param_ptr);
        switch (type)
        {
            case FloatParam:
                resetParameter (*reinterpret_cast<FloatParameter*> (param_ptr.get_ptr()));
                break;
            case ChoiceParam:
                resetParameter (*reinterpret_cast<ChoiceParameter*> (param_ptr.get_ptr()));
                break;
            case BoolParam:
                resetParameter (*reinterpret_cast<BoolParameter*> (param_ptr.get_ptr()));
                break;
            default:
                break;
        }
    }
    */
}

inline json ParamHolder::serialize_json (const ParamHolder& paramHolder)
{
    auto serial = nlohmann::json::object();
    paramHolder.doForAllParameters (
        [&serial] (auto& param, size_t)
        {
            const auto paramID = toStringView (param.paramID);
            serial[paramID] = ParameterTypeHelpers::getValue (param);
        });
    return serial;
}

inline void ParamHolder::deserialize_json (const json& deserial, ParamHolder& paramHolder)
{
    paramHolder.doForAllParameters (
        [&deserial] (auto& param, size_t)
        {
            const auto paramID = toStringView (param.paramID);
            ParameterTypeHelpers::setValue (deserial.value (paramID, ParameterTypeHelpers::getDefaultValue (param)), param);
        });
}

inline void ParamHolder::legacy_deserialize (const json& deserial, ParamHolder& paramHolder)
{
    using Serializer = JSONSerializer;
    std::vector<std::string_view> paramIDsThatHaveBeenDeserialized {};
    if (const auto numParamIDsAndVals = Serializer::getNumChildElements (deserial); numParamIDsAndVals % 2 == 0)
    {
        paramIDsThatHaveBeenDeserialized.reserve (static_cast<size_t> (numParamIDsAndVals) / 2);
        for (int i = 0; i < numParamIDsAndVals; i += 2)
        {
            const auto paramID = Serializer::getChildElement (deserial, i).get<std::string_view>();
            const auto& paramDeserial = Serializer::getChildElement (deserial, i + 1);

            paramHolder.doForAllParameters (
                [&] (auto& param, size_t)
                {
                    if (toStringView (param.paramID) != paramID)
                        return;

                    paramIDsThatHaveBeenDeserialized.push_back (paramID);
                    ParameterTypeHelpers::deserializeParameter<Serializer> (paramDeserial, param);
                });
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
