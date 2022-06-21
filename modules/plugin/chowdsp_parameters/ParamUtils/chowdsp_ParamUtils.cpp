#include "chowdsp_ParamUtils.h"

namespace chowdsp::ParamUtils
{
juce::String freqValToString (float freqVal)
{
    if (freqVal <= 1000.0f)
        return juce::String (freqVal, 2, false) + " Hz";

    return juce::String (freqVal / 1000.0f, 2, false) + " kHz";
}

float stringToFreqVal (const juce::String& s)
{
    auto freqVal = s.getFloatValue();

    if (s.getLastCharacter() == 'k')
        freqVal *= 1000.0f;

    return freqVal;
}

juce::String percentValToString (float percentVal)
{
    juce::String percentStr = juce::String (int (percentVal * 100.0f));
    return percentStr + "%";
}

float stringToPercentVal (const juce::String& s) { return s.getFloatValue() / 100.0f; }

juce::String gainValToString (float gainVal)
{
    juce::String gainStr = juce::String (gainVal, 2, false);
    return gainStr + " dB";
}

float stringToGainVal (const juce::String& s) { return s.getFloatValue(); }

juce::String ratioValToString (float ratioVal)
{
    juce::String ratioStr = juce::String (ratioVal, 2, false);
    return ratioStr + " : 1";
}

float stringToRatioVal (const juce::String& s) { return s.getFloatValue(); }

juce::String timeMsValToString (float timeMsVal)
{
    if (timeMsVal < 1000.0f)
        return juce::String (timeMsVal, 2, false) + " ms";

    juce::String timeSecStr = juce::String (timeMsVal / 1000.0f, 2, false);
    return timeSecStr + " s";
}

float stringToTimeMsVal (const juce::String& s) { return s.getFloatValue(); }

juce::String floatValToString (float floatVal)
{
    return floatValToStringDecimal<2> (floatVal);
}

float stringToFloatVal (const juce::String& s) { return s.getFloatValue(); }

void createFreqParameter (Parameters& params, const juce::String& id, const juce::String& name, float min, float max, float centre, float defaultValue)
{
    auto freqRange = createNormalisableRange (min, max, centre);
    emplace_param<FloatParameter> (params, id, name, freqRange, defaultValue, &freqValToString, &stringToFreqVal);
}

void createPercentParameter (Parameters& params, const juce::String& id, const juce::String& name, float defaultValue)
{
    emplace_param<FloatParameter> (params, id, name, juce::NormalisableRange { 0.0f, 1.0f }, defaultValue, &percentValToString, &stringToPercentVal);
}

void createBipolarPercentParameter (Parameters& params, const juce::String& id, const juce::String& name, float defaultValue)
{
    emplace_param<FloatParameter> (params, id, name, juce::NormalisableRange { -1.0f, 1.0f }, defaultValue, &percentValToString, &stringToPercentVal);
}

void createGainDBParameter (Parameters& params, const juce::String& id, const juce::String& name, float min, float max, float defaultValue, float centerValue)
{
    juce::NormalisableRange<float> range { min, max };
    if (centerValue > -1000.0f)
        range.setSkewForCentre (centerValue);

    emplace_param<FloatParameter> (params, id, name, range, defaultValue, &gainValToString, &stringToGainVal);
}

void createTimeMsParameter (Parameters& params, const juce::String& id, const juce::String& name, const juce::NormalisableRange<float>& range, float defaultValue)
{
    emplace_param<FloatParameter> (params, id, name, range, defaultValue, &timeMsValToString, &stringToTimeMsVal);
}

void createRatioParameter (Parameters& params, const juce::String& id, const juce::String& name, const juce::NormalisableRange<float>& range, float defaultValue)
{
    emplace_param<FloatParameter> (params, id, name, range, defaultValue, &ratioValToString, &stringToRatioVal);
}

} // namespace chowdsp::ParamUtils
