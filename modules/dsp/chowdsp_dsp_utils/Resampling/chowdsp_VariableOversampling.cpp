#include "chowdsp_VariableOversampling.h"

namespace
{
const juce::String oneXString = "1x";
const juce::String twoXString = "2x";
const juce::String fourXString = "4x";
const juce::String eightXString = "8x";
const juce::String sixteenXString = "16x";

const juce::String minPhaseString = "Min. Phase";
const juce::String linPhaseString = "Linear Phase";

const juce::String factorSuffix = "_factor";
const juce::String modeSuffix = "_mode";
const juce::String renderFactorSuffix = "_render_factor";
const juce::String renderModeSuffix = "_render_mode";
const juce::String renderLikeRealtimeSuffix = "_render_like_realtime";
} // namespace

namespace chowdsp
{
template <typename FloatType>
typename VariableOversampling<FloatType>::OSFactor VariableOversampling<FloatType>::stringToOSFactor (const juce::String& factorStr)
{
    if (factorStr == oneXString)
        return OSFactor::OneX;
    if (factorStr == twoXString)
        return OSFactor::TwoX;
    if (factorStr == fourXString)
        return OSFactor::FourX;
    if (factorStr == eightXString)
        return OSFactor::EightX;
    if (factorStr == sixteenXString)
        return OSFactor::SixteenX;

    jassertfalse; // unknown OS factor
    return OSFactor::OneX;
}

template <typename FloatType>
typename VariableOversampling<FloatType>::OSMode VariableOversampling<FloatType>::stringToOSMode (const juce::String& modeStr)
{
    if (modeStr == minPhaseString)
        return OSMode::MinPhase;
    if (modeStr == linPhaseString)
        return OSMode::LinPhase;

    jassertfalse; // unknown OS mode
    return OSMode::MinPhase;
}

template <typename FloatType>
juce::String VariableOversampling<FloatType>::osFactorToString (OSFactor factor)
{
    switch (factor)
    {
        case OSFactor::OneX:
            return oneXString;
        case OSFactor::TwoX:
            return twoXString;
        case OSFactor::FourX:
            return fourXString;
        case OSFactor::EightX:
            return eightXString;
        case OSFactor::SixteenX:
            return sixteenXString;
    }

    jassertfalse; // unknown OS factor
    return {};
}

template <typename FloatType>
juce::String VariableOversampling<FloatType>::osModeToString (OSMode mode)
{
    switch (mode)
    {
        case OSMode::MinPhase:
            return minPhaseString;
        case OSMode::LinPhase:
            return linPhaseString;
    }

    jassertfalse; // unknown OS mode
    return {};
}

template <typename FloatType>
VariableOversampling<FloatType>::VariableOversampling (const juce::AudioProcessorValueTreeState& vts, bool useIntegerLatency, const juce::String& prefix)
    : proc (vts.processor),
      usingIntegerLatency (useIntegerLatency)
{
    auto loadParamPointer = [&vts, &prefix] (auto& param, const juce::String& paramSuffix)
    {
        using ParamType = std::remove_reference_t<decltype (param)>;
        param = dynamic_cast<ParamType> (vts.getParameter (prefix + paramSuffix));

        if (paramSuffix == factorSuffix || paramSuffix == modeSuffix)
            jassert (param != nullptr); // either createParameterLayout was not called, or paramPrefix is incorrect!
    };

    loadParamPointer (osParam, factorSuffix);
    loadParamPointer (osModeParam, modeSuffix);
    loadParamPointer (osOfflineParam, renderFactorSuffix);
    loadParamPointer (osOfflineModeParam, renderModeSuffix);
    loadParamPointer (osOfflineSameParam, renderLikeRealtimeSuffix);

    numOSChoices = osParam->choices.size();
}

template <typename FloatType>
void VariableOversampling<FloatType>::createParameterLayout (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& params,
                                                             OSFactor defaultFactor,
                                                             OSMode defaultMode,
                                                             int versionHint,
                                                             bool includeRenderOptions,
                                                             const juce::String& paramPrefix)
{
    createParameterLayout (params,
                           { OSFactor::OneX, OSFactor::TwoX, OSFactor::FourX, OSFactor::EightX, OSFactor::SixteenX },
                           { OSMode::MinPhase, OSMode::LinPhase },
                           defaultFactor,
                           defaultMode,
                           versionHint,
                           includeRenderOptions,
                           paramPrefix);
}

template <typename FloatType>
void VariableOversampling<FloatType>::createParameterLayout (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& params,
                                                             std::initializer_list<OSFactor> osFactors,
                                                             std::initializer_list<OSMode> osModes,
                                                             OSFactor defaultFactor,
                                                             OSMode defaultMode,
                                                             int versionHint,
                                                             bool includeRenderOptions,
                                                             const juce::String& paramPrefix)
{
    juce::StringArray osFactorChoices;
    int defaultOSFactor = 0;
    for (auto factor : osFactors)
    {
        if (factor == defaultFactor)
            defaultOSFactor = osFactorChoices.size();

        osFactorChoices.add (osFactorToString (factor));
    }

    juce::StringArray osModeChoices;
    int defaultOSMode = 0;
    for (auto mode : osModes)
    {
        if (mode == defaultMode)
            defaultOSMode = osModeChoices.size();

        osModeChoices.add (osModeToString (mode));
    }

    auto getParamID = [&paramPrefix, versionHint] (const juce::String& paramSuffix)
    {
#if JUCE_VERSION < 0x070000
        juce::ignoreUnused (versionHint);
        return paramPrefix + paramSuffix;
#else
        return juce::ParameterID { paramPrefix + paramSuffix, versionHint };
#endif
    };

    params.push_back (std::make_unique<juce::AudioParameterChoice> (getParamID (factorSuffix), "Oversampling Factor", osFactorChoices, defaultOSFactor));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (getParamID (modeSuffix), "Oversampling Mode", osModeChoices, defaultOSMode));

    if (includeRenderOptions)
    {
        params.push_back (std::make_unique<juce::AudioParameterChoice> (getParamID (renderFactorSuffix), "Oversampling Factor (render)", osFactorChoices, defaultOSFactor));
        params.push_back (std::make_unique<juce::AudioParameterChoice> (getParamID (renderModeSuffix), "Oversampling Mode (render)", osModeChoices, defaultOSMode));
        params.push_back (std::make_unique<juce::AudioParameterBool> (getParamID (renderLikeRealtimeSuffix), "Oversampling (render like real-time)", true));
    }
}

template <typename FloatType>
bool VariableOversampling<FloatType>::updateOSFactor()
{
    curOS = getOSIndex (*osParam, *osModeParam);
    if (proc.isNonRealtime() && osOfflineParam != nullptr && ! *osOfflineSameParam)
    {
        curOS = getOSIndex (*osOfflineParam, *osOfflineModeParam);
    }

    if (curOS != prevOS)
    {
        prevOS = curOS;
        return true;
    }

    return false;
}

template <typename FloatType>
void VariableOversampling<FloatType>::prepareToPlay (double sr, int samplesPerBlock, int numChannels)
{
    oversamplers.clear();

    for (const auto& modeStr : osModeParam->choices)
    {
        auto osMode = stringToOSMode (modeStr);
        auto filterType = osMode == OSMode::LinPhase ? juce::dsp::Oversampling<FloatType>::filterHalfBandFIREquiripple
                                                     : juce::dsp::Oversampling<FloatType>::filterHalfBandPolyphaseIIR;

        for (const auto& factorStr : osParam->choices)
        {
            auto osFactor = stringToOSFactor (factorStr);
            oversamplers.add (std::make_unique<juce::dsp::Oversampling<FloatType>> (numChannels, static_cast<int> (osFactor), filterType, true, usingIntegerLatency));
        }
    }

    for (auto& os : oversamplers)
        os->initProcessing ((size_t) samplesPerBlock);

    sampleRate = (float) sr;
    curOS = getOSIndex (*osParam, *osModeParam);
    prevOS = curOS;

    sampleRateOrBlockSizeChangedBroadcaster();
}

template <typename FloatType>
void VariableOversampling<FloatType>::reset()
{
    for (auto& os : oversamplers)
        os->reset();
}

template <typename FloatType>
float VariableOversampling<FloatType>::getLatencySamples() const noexcept
{
    jassert (hasBeenPrepared()); // Make sure to prepare the oversampler before calling this!

    return (float) oversamplers[curOS]->getLatencyInSamples();
}

template <typename FloatType>
float VariableOversampling<FloatType>::getLatencyMilliseconds (int osIndex) const noexcept
{
    jassert (hasBeenPrepared()); // Make sure to prepare the oversampler before calling this!

    osIndex = osIndex < 0 ? curOS : osIndex;
    jassert (osIndex < oversamplers.size()); // Make sure that osIndex is in range!

    return ((float) oversamplers[osIndex]->getLatencyInSamples() / sampleRate) * 1000.0f;
}

template class VariableOversampling<float>;
template class VariableOversampling<double>;
} // namespace chowdsp
