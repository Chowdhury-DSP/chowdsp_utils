#include "chowdsp_VariableOversampling.h"

namespace chowdsp
{
template <typename FloatType>
typename VariableOversampling<FloatType>::OSFactor VariableOversampling<FloatType>::stringToOSFactor (const juce::String& factorStr)
{
    if (factorStr == "1x")
        return OSFactor::OneX;
    if (factorStr == "2x")
        return OSFactor::TwoX;
    if (factorStr == "4x")
        return OSFactor::FourX;
    if (factorStr == "8x")
        return OSFactor::EightX;
    if (factorStr == "16x")
        return OSFactor::SixteenX;

    jassertfalse; // unknown OS factor
    return OSFactor::OneX;
}

template <typename FloatType>
typename VariableOversampling<FloatType>::OSMode VariableOversampling<FloatType>::stringToOSMode (const juce::String& modeStr)
{
    if (modeStr == "Min. Phase")
        return OSMode::MinPhase;
    if (modeStr == "Linear Phase")
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
            return "1x";
        case OSFactor::TwoX:
            return "2x";
        case OSFactor::FourX:
            return "4x";
        case OSFactor::EightX:
            return "8x";
        case OSFactor::SixteenX:
            return "16x";
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
            return "Min. Phase";
        case OSMode::LinPhase:
            return "Linear Phase";
    }

    jassertfalse; // unknown OS mode
    return {};
}

template <typename FloatType>
VariableOversampling<FloatType>::VariableOversampling (juce::AudioProcessorValueTreeState& vts, bool useIntegerLatency, const juce::String& prefix) : proc (vts.processor),
                                                                                                                                                      usingIntegerLatency (useIntegerLatency),
                                                                                                                                                      paramPrefix (prefix)
{
    osParam = dynamic_cast<juce::AudioParameterChoice*> (vts.getParameter (paramPrefix + "_factor"));
    osModeParam = dynamic_cast<juce::AudioParameterChoice*> (vts.getParameter (paramPrefix + "_mode"));
    osOfflineParam = dynamic_cast<juce::AudioParameterChoice*> (vts.getParameter (paramPrefix + "_render_factor"));
    osOfflineModeParam = dynamic_cast<juce::AudioParameterChoice*> (vts.getParameter (paramPrefix + "_render_mode"));
    osOfflineSameParam = dynamic_cast<juce::AudioParameterBool*> (vts.getParameter (paramPrefix + "_render_like_realtime"));

    // either createParameterLayout was not called, or paramPrefix is incorrect!
    jassert (osParam != nullptr);

    numOSChoices = osParam->choices.size();
}

template <typename FloatType>
void VariableOversampling<FloatType>::createParameterLayout (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& params,
                                                             OSFactor defaultFactor,
                                                             OSMode defaultMode,
                                                             bool includeRenderOptions,
                                                             const juce::String& paramPrefix)
{
    createParameterLayout (params,
                           { OSFactor::OneX, OSFactor::TwoX, OSFactor::FourX, OSFactor::EightX, OSFactor::SixteenX },
                           { OSMode::MinPhase, OSMode::LinPhase },
                           defaultFactor,
                           defaultMode,
                           includeRenderOptions,
                           paramPrefix);
}

template <typename FloatType>
void VariableOversampling<FloatType>::createParameterLayout (std::vector<std::unique_ptr<juce::RangedAudioParameter>>& params,
                                                             std::initializer_list<OSFactor> osFactors,
                                                             std::initializer_list<OSMode> osModes,
                                                             OSFactor defaultFactor,
                                                             OSMode defaultMode,
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

    params.push_back (std::make_unique<juce::AudioParameterChoice> (paramPrefix + "_factor", "Oversampling Factor", osFactorChoices, defaultOSFactor));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (paramPrefix + "_mode", "Oversampling Mode", osModeChoices, defaultOSMode));

    if (includeRenderOptions)
    {
        params.push_back (std::make_unique<juce::AudioParameterChoice> (paramPrefix + "_render_factor", "Oversampling Factor (render)", osFactorChoices, defaultOSFactor));
        params.push_back (std::make_unique<juce::AudioParameterChoice> (paramPrefix + "_render_mode", "Oversampling Mode (render)", osModeChoices, defaultOSMode));
        params.push_back (std::make_unique<juce::AudioParameterBool> (paramPrefix + "_render_like_realtime", "Oversampling (render like real-time)", true));
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

    listeners.call (&Listener::sampleRateOrBlockSizeChanged);
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
