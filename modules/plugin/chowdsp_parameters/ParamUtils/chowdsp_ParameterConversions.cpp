#include "chowdsp_ParameterConversions.h"

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

    if (s.getLastCharacter() == 'k' || s.endsWith ("kHz") || s.endsWith ("khz"))
        freqVal *= 1000.0f;

    return freqVal;
}

juce::String percentValToString (float percentVal)
{
    auto percentStr = juce::String (int (percentVal * 100.0f));
    return percentStr + "%";
}

float stringToPercentVal (const juce::String& s) { return s.getFloatValue() / 100.0f; }

juce::String gainValToString (float gainVal)
{
    auto gainStr = juce::String (gainVal, 2, false);
    return gainStr + " dB";
}

float stringToGainVal (const juce::String& s) { return s.getFloatValue(); }

juce::String ratioValToString (float ratioVal)
{
    auto ratioStr = juce::String (ratioVal, 2, false);
    return ratioStr + " : 1";
}

float stringToRatioVal (const juce::String& s) { return s.getFloatValue(); }

juce::String timeMsValToString (float timeMsVal)
{
    if (timeMsVal < 1000.0f)
        return juce::String (timeMsVal, 2, false) + " ms";

    auto timeSecStr = juce::String (timeMsVal / 1000.0f, 2, false);
    return timeSecStr + " s";
}

float stringToTimeMsVal (const juce::String& s) { return s.getFloatValue(); }

juce::String floatValToString (float floatVal)
{
    return floatValToStringDecimal<2> (floatVal);
}

float stringToFloatVal (const juce::String& s) { return s.getFloatValue(); }
} // namespace chowdsp::ParamUtils
