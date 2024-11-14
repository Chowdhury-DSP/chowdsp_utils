#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
namespace chowdsp::EQ
{
template <size_t NumBands>
StandardEQParameters<NumBands>::StandardEQParameters (EQParameterHandles&& paramHandles,
                                                      const juce::String& paramHolderName)
    : ParamHolder (paramHolderName),
      eqParams (std::move (paramHandles))
{
    for (auto& bandParams : eqParams)
    {
        bandParams.paramHolder.add (bandParams.onOffParam,
                                    bandParams.typeParam,
                                    bandParams.freqParam,
                                    bandParams.qParam,
                                    bandParams.gainParam);
        add (bandParams.paramHolder);
    }
}

template <size_t NumBands>
typename StandardEQParameters<NumBands>::Params StandardEQParameters<NumBands>::getEQParameters (const EQParameterHandles& paramHandles)
{
    Params params {};
    for (size_t i = 0; i < NumBands; ++i)
    {
        params.bands[i] = { typename Params::BandParams {
            paramHandles[i].freqParam->getCurrentValue(),
            paramHandles[i].qParam->getCurrentValue(),
            paramHandles[i].gainParam->getCurrentValue(),
            paramHandles[i].typeParam->getIndex(),
            paramHandles[i].onOffParam->get(),
        } };
    }

    return params;
}

template <size_t NumBands>
template <typename EQType>
void StandardEQParameters<NumBands>::setEQParameters (EQType& eq, const Params& params)
{
    Params::setEQParameters (eq, params);
}

template <size_t NumBands>
void StandardEQParameters<NumBands>::loadEQParameters (const Params& eqParams, StandardEQParameters& stateParams)
{
    for (size_t i = 0; i < NumBands; ++i)
    {
        const auto& bandParams = eqParams.bands[i].params;

        ParameterTypeHelpers::setValue (bandParams.bandOnOff, *stateParams.eqParams[i].onOffParam);
        ParameterTypeHelpers::setValue (bandParams.bandType, *stateParams.eqParams[i].typeParam);
        ParameterTypeHelpers::setValue (bandParams.bandFreqHz, *stateParams.eqParams[i].freqParam);
        ParameterTypeHelpers::setValue (bandParams.bandQ, *stateParams.eqParams[i].qParam);
        ParameterTypeHelpers::setValue (bandParams.bandGainDB, *stateParams.eqParams[i].gainParam);
    }
}
} // namespace chowdsp::EQ
#else
namespace chowdsp::EQ
{
#ifndef DOXYGEN
namespace eqparams_detail
{
    inline auto getTagForBand (const juce::String& paramPrefix, int bandIndex, const juce::String& tag)
    {
        return paramPrefix + "band" + juce::String (bandIndex) + "_" + tag;
    }

#if JUCE_VERSION < 0x070000
    inline auto getParamIDForBand (const juce::String& paramPrefix, int bandIndex, const juce::String& tag, int versionHint)
    {
        juce::ignoreUnused (versionHint);
        return getTagForBand (paramPrefix, bandIndex, tag);
    }
#else
    inline auto getParamIDForBand (const juce::String& paramPrefix, int bandIndex, const juce::String& tag, int versionHint)
    {
        return ParameterID { getTagForBand (paramPrefix, bandIndex, tag), versionHint };
    }
#endif

    inline juce::String getNameForBand (int bandIndex, const juce::String& name)
    {
        return "Band " + juce::String (bandIndex) + " " + name;
    }

    inline static const juce::String eqBandFreqTag = "eq_band_freq";
    inline static const juce::String eqBandQTag = "eq_band_q";
    inline static const juce::String eqBandGainTag = "eq_band_gain";
    inline static const juce::String eqBandTypeTag = "eq_band_type";
    inline static const juce::String eqBandOnOffTag = "eq_band_on_off";

    // corresponding filter type choices for chowdsp::EQ::DefaultEQBand
    inline static const auto defaultEQBandTypeChoices = juce::StringArray {
        "1-Pole HPF",
        "2-Pole HPF",
        "Low-Shelf",
        "Bell",
        "Notch",
        "High-Shelf",
        "1-Pole LPF",
        "2-Pole LPF",
    };
} // namespace eqparams_detail
#endif // DOXYGEN

template <size_t NumBands>
void StandardEQParameters<NumBands>::initialiseEQParameters (juce::AudioProcessorValueTreeState& vts, EQParameterHandles& params, const juce::String& paramPrefix)
{
    using namespace eqparams_detail;
    for (size_t i = 0; i < NumBands; ++i)
    {
        using namespace chowdsp::ParamUtils;
        loadParameterPointer (std::get<FREQ> (params[i]), vts, getTagForBand (paramPrefix, (int) i, eqBandFreqTag));
        loadParameterPointer (std::get<Q> (params[i]), vts, getTagForBand (paramPrefix, (int) i, eqBandQTag));
        loadParameterPointer (std::get<GAIN> (params[i]), vts, getTagForBand (paramPrefix, (int) i, eqBandGainTag));
        loadParameterPointer (std::get<TYPE> (params[i]), vts, getTagForBand (paramPrefix, (int) i, eqBandTypeTag));
        loadParameterPointer (std::get<ONOFF> (params[i]), vts, getTagForBand (paramPrefix, (int) i, eqBandOnOffTag));
    }
}

template <size_t NumBands>
void StandardEQParameters<NumBands>::addEQParameters (Parameters& params, const juce::String& paramPrefix, juce::StringArray eqBandTypeChoices, int defaultEQBandTypeChoice, int versionHint)
{
    using namespace eqparams_detail;
    using namespace chowdsp::ParamUtils;

    if (eqBandTypeChoices.isEmpty())
    {
        eqBandTypeChoices = defaultEQBandTypeChoices;
        defaultEQBandTypeChoice = eqBandTypeChoices.indexOf ("Bell");
    }

    // defaultEQBandChoice must be a valid index into eqBandTypeChoices
    jassert (juce::isPositiveAndBelow (defaultEQBandTypeChoice, eqBandTypeChoices.size()));

    auto addQParam = [&params] (const ParameterID& paramID, const juce::String& name)
    {
        emplace_param<FloatParameter> (params, paramID, name, createNormalisableRange (0.1f, 10.0f, 0.7071f), 0.7071f, &floatValToString, &stringToFloatVal);
    };

    for (int i = 0; i < (int) NumBands; ++i)
    {
        emplace_param<BoolParameter> (params, getParamIDForBand (paramPrefix, i, eqBandOnOffTag, versionHint), getNameForBand (i, "On/Off"), false);
        emplace_param<ChoiceParameter> (params, getParamIDForBand (paramPrefix, i, eqBandTypeTag, versionHint), getNameForBand (i, "Type"), eqBandTypeChoices, defaultEQBandTypeChoice);
        createFreqParameter (params, getParamIDForBand (paramPrefix, i, eqBandFreqTag, versionHint), getNameForBand (i, "Freq."), 20.0f, 20000.0f, 2000.0f, 1000.0f);
        addQParam (getParamIDForBand (paramPrefix, i, eqBandQTag, versionHint), getNameForBand (i, "Q"));
        createGainDBParameter (params, getParamIDForBand (paramPrefix, i, eqBandGainTag, versionHint), getNameForBand (i, "Gain"), -18.0f, 18.0f, 0.0f);
    }
}

template <size_t NumBands>
typename StandardEQParameters<NumBands>::Params StandardEQParameters<NumBands>::getEQParameters (const EQParameterHandles& paramHandles)
{
    Params params {};
    for (size_t i = 0; i < NumBands; ++i)
    {
        params.bands[i] = { typename Params::BandParams {
            std::get<ParameterType::FREQ> (paramHandles[i])->getCurrentValue(),
            std::get<ParameterType::Q> (paramHandles[i])->getCurrentValue(),
            std::get<ParameterType::GAIN> (paramHandles[i])->getCurrentValue(),
            std::get<ParameterType::TYPE> (paramHandles[i])->getIndex(),
            std::get<ParameterType::ONOFF> (paramHandles[i])->get(),
        } };
    }

    return params;
}

template <size_t NumBands>
template <typename EQType>
void StandardEQParameters<NumBands>::setEQParameters (EQType& eq, const Params& params)
{
    Params::setEQParameters (eq, params);
}

template <size_t NumBands>
void StandardEQParameters<NumBands>::loadEQParameters (const Params& params, juce::AudioProcessorValueTreeState& vts, const juce::String& paramPrefix)
{
    auto setParameter = [] (auto* param, float newValue)
    {
        param->beginChangeGesture();
        param->setValueNotifyingHost (param->convertTo0to1 (newValue));
        param->endChangeGesture();
    };

    using namespace eqparams_detail;
    for (size_t i = 0; i < NumBands; ++i)
    {
        const auto& bandParams = params.bands[i].params;

        setParameter (vts.getParameter (getTagForBand (paramPrefix, (int) i, eqBandFreqTag)), bandParams.bandFreqHz);
        setParameter (vts.getParameter (getTagForBand (paramPrefix, (int) i, eqBandQTag)), bandParams.bandQ);
        setParameter (vts.getParameter (getTagForBand (paramPrefix, (int) i, eqBandGainTag)), bandParams.bandGainDB);
        setParameter (vts.getParameter (getTagForBand (paramPrefix, (int) i, eqBandTypeTag)), (float) bandParams.bandType);
        setParameter (vts.getParameter (getTagForBand (paramPrefix, (int) i, eqBandOnOffTag)), bandParams.bandOnOff);
    }
}
} // namespace chowdsp::EQ
#endif
