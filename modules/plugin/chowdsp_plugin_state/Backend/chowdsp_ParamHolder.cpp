namespace chowdsp
{
inline ParamHolder::ParamHolder (const juce::String& phName, bool phIsOwning)
    : name (phName),
      isOwning (phIsOwning)
{
    juce::ignoreUnused (isOwning);
}

template <typename ParamType, typename... OtherParams>
std::enable_if_t<std::is_base_of_v<FloatParameter, ParamType>, void>
    ParamHolder::add (OptionalPointer<ParamType>& floatParam, OtherParams&... others)
{
    auto& param = floatParams.emplace_back (isOwning ? floatParam.release() : floatParam.get(), isOwning);
    allParamsMap.insert ({ param->paramID.toStdString(), param.get() });
    add (others...);
}

template <typename ParamType, typename... OtherParams>
std::enable_if_t<std::is_base_of_v<ChoiceParameter, ParamType>, void>
    ParamHolder::add (OptionalPointer<ParamType>& choiceParam, OtherParams&... others)
{
    auto& param = choiceParams.emplace_back (isOwning ? choiceParam.release() : choiceParam.get(), isOwning);
    allParamsMap.insert ({ param->paramID.toStdString(), param.get() });
    add (others...);
}

template <typename ParamType, typename... OtherParams>
std::enable_if_t<std::is_base_of_v<BoolParameter, ParamType>, void>
    ParamHolder::add (OptionalPointer<ParamType>& boolParam, OtherParams&... others)
{
    auto& param = boolParams.emplace_back (isOwning ? boolParam.release() : boolParam.get(), isOwning);
    allParamsMap.insert ({ param->paramID.toStdString(), param.get() });
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
    otherParams.push_back (&paramHolder);
    allParamsMap.merge (paramHolder.allParamsMap);
    jassert (paramHolder.allParamsMap.empty()); // assuming no duplicate parameter IDs, all the parameters should be moved in the merge!
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
    auto numParams = int (floatParams.size() + choiceParams.size() + boolParams.size());
    for (const auto& holder : otherParams)
        numParams += holder->count();
    return numParams;
}

inline void ParamHolder::clear()
{
    // It's generally not safe to clear the parameters if this is an owning ParamHolder
    // since we're almost certainly leaving some dangling references lying around!
    jassert (! isOwning);

    allParamsMap.clear();
    floatParams.clear();
    choiceParams.clear();
    boolParams.clear();
    otherParams.clear();
}

template <typename ParamContainersCallable, typename ParamHolderCallable>
void ParamHolder::doForAllParameterContainers (ParamContainersCallable&& paramContainersCallable, ParamHolderCallable&& paramHolderCallable)
{
    paramContainersCallable (floatParams);
    paramContainersCallable (choiceParams);
    paramContainersCallable (boolParams);
    for (auto& holder : otherParams)
        paramHolderCallable (*holder);
}

template <typename ParamContainersCallable, typename ParamHolderCallable>
void ParamHolder::doForAllParameterContainers (ParamContainersCallable&& paramContainersCallable, ParamHolderCallable&& paramHolderCallable) const
{
    paramContainersCallable (floatParams);
    paramContainersCallable (choiceParams);
    paramContainersCallable (boolParams);
    for (const auto& holder : otherParams)
        paramHolderCallable (*holder);
}

inline void ParamHolder::connectParametersToProcessor (juce::AudioProcessor& processor)
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
size_t ParamHolder::doForAllParameters (Callable&& callable, size_t index)
{
    doForAllParameterContainers (
        [&index, call = std::forward<Callable> (callable)] (auto& paramVec)
        {
            for (auto& param : paramVec)
            {
                call (*param, index);
                index++;
            }
        },
        [&index, call = std::forward<Callable> (callable)] (auto& holder)
        {
            index = holder.doForAllParameters (std::move (call), index);
        });

    return index;
}

template <typename Callable>
size_t ParamHolder::doForAllParameters (Callable&& callable, size_t index) const
{
    doForAllParameterContainers (
        [&index, call = std::forward<Callable> (callable)] (const auto& paramVec)
        {
            for (const auto& param : paramVec)
            {
                call (*param, index);
                index++;
            }
        },
        [&index, call = std::forward<Callable> (callable)] (const auto& holder)
        {
            index = holder.doForAllParameters (std::move (call), index);
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
            [&paramDeserial] (const ParamPtrVariant& paramPtr)
            {
                const auto deserializeParam = [] (auto* param, auto& pd)
                {
                    ParameterTypeHelpers::deserializeParameter<Serializer> (pd, *param);
                };

                if (auto* floatParamPtr = std::get_if<FloatParameter*> (&paramPtr))
                    deserializeParam (*floatParamPtr, paramDeserial);
                else if (auto* choiceParamPtr = std::get_if<ChoiceParameter*> (&paramPtr))
                    deserializeParam (*choiceParamPtr, paramDeserial);
                else if (auto* boolParamPtr = std::get_if<BoolParameter*> (&paramPtr))
                    deserializeParam (*boolParamPtr, paramDeserial);
                else
                    jassertfalse; // bad variant access?
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

    doForAllParameterContainers (
        [] (auto&) {},
        [&version] (auto& holder)
        {
            holder.applyVersionStreaming (version);
        });
}
} // namespace chowdsp
