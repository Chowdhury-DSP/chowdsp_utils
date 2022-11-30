#include "chowdsp_ParamUtils.h"

namespace chowdsp::ParamUtils
{
void createFreqParameter (Parameters& params, const ParameterID& id, const juce::String& name, float min, float max, float centre, float defaultValue)
{
    auto freqRange = createNormalisableRange (min, max, centre);
    emplace_param<FreqHzParameter> (params, id, name, freqRange, defaultValue);
}

void createPercentParameter (Parameters& params, const ParameterID& id, const juce::String& name, float defaultValue)
{
    emplace_param<PercentParameter> (params, id, name, defaultValue);
}

void createBipolarPercentParameter (Parameters& params, const ParameterID& id, const juce::String& name, float defaultValue)
{
    emplace_param<PercentParameter> (params, id, name, defaultValue, true);
}

void createGainDBParameter (Parameters& params, const ParameterID& id, const juce::String& name, float min, float max, float defaultValue, float centerValue)
{
    juce::NormalisableRange range { min, max };
    if (centerValue > -1000.0f)
        range.setSkewForCentre (centerValue);

    emplace_param<GainDBParameter> (params, id, name, range, defaultValue);
}

void createTimeMsParameter (Parameters& params, const ParameterID& id, const juce::String& name, const juce::NormalisableRange<float>& range, float defaultValue)
{
    emplace_param<FloatParameter> (params, id, name, range, defaultValue, &timeMsValToString, &stringToTimeMsVal);
}

void createRatioParameter (Parameters& params, const ParameterID& id, const juce::String& name, const juce::NormalisableRange<float>& range, float defaultValue)
{
    emplace_param<FloatParameter> (params, id, name, range, defaultValue, &ratioValToString, &stringToRatioVal);
}

} // namespace chowdsp::ParamUtils
