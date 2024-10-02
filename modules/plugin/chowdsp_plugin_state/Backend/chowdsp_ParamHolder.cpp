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
    floatParams.emplace_back (isOwning ? floatParam.release() : floatParam.get(), isOwning);
    add (others...);
}

template <typename ParamType, typename... OtherParams>
std::enable_if_t<std::is_base_of_v<ChoiceParameter, ParamType>, void>
    ParamHolder::add (OptionalPointer<ParamType>& choiceParam, OtherParams&... others)
{
    choiceParams.emplace_back (isOwning ? choiceParam.release() : choiceParam.get(), isOwning);
    add (others...);
}

template <typename ParamType, typename... OtherParams>
std::enable_if_t<std::is_base_of_v<BoolParameter, ParamType>, void>
    ParamHolder::add (OptionalPointer<ParamType>& boolParam, OtherParams&... others)
{
    boolParams.emplace_back (isOwning ? boolParam.release() : boolParam.get(), isOwning);
    add (others...);
}

template <typename ParamType, typename... OtherParams>
std::enable_if_t<std::is_base_of_v<ParamHolder, ParamType>, void>
    ParamHolder::add (ParamType& paramHolder, OtherParams&... others)
{
    otherParams.push_back (&paramHolder);
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
    auto serial = Serializer::createBaseElement(paramHolder.name);
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

    juce::StringArray paramIDsThatHaveBeenDeserialized {};
    if (const auto numParamIDsAndVals = Serializer::getNumAttributes (deserial))
    {
        for (int i = 0; i < numParamIDsAndVals; i += 1)
        {
            juce::String paramID {};
            paramID = Serializer::getAttributeName (deserial, i);
//DBG("PArAMID" + paramID);
            //Serialization::deserialize<Serializer> (, paramID);
//const auto paramDeserial = Serializer::getAttribute (deserial,paramID);


            paramHolder.doForAllParameters (
                [&deserial,
                 &paramID = std::as_const (paramID),
                 &paramIDsThatHaveBeenDeserialized] (auto& param, size_t)
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
       // jassertfalse; // state loading error
    }

    for(auto id: paramIDsThatHaveBeenDeserialized)
    {
        DBG("deserialzied " + id);
    }
    // set all un-matched objects to their default values
    paramHolder.doForAllParameters (
        [&paramIDsThatHaveBeenDeserialized] (auto& param, size_t)
        {
            if (! paramIDsThatHaveBeenDeserialized.contains (param.paramID))
                ParameterTypeHelpers::resetParameter (param);
        });
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
