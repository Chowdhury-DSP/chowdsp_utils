#pragma once

namespace chowdsp::ParamUtils
{
juce::String freqValToString (float freqVal);
float stringToFreqVal (const juce::String& s);

juce::String percentValToString (float percentVal);
float stringToPercentVal (const juce::String& s);

juce::String gainValToString (float gainVal);
float stringToGainVal (const juce::String& s);

juce::String ratioValToString (float ratioVal);
float stringToRatioVal (const juce::String& s);

juce::String timeMsValToString (float timeMsVal);
float stringToTimeMsVal (const juce::String& s);

juce::String semitonesValToString (float semitonesVal, bool snapToInt);
float stringToSemitonesVal (const juce::String& s);

juce::String floatValToString (float floatVal);
template <int NumDecimalPlaces>
juce::String floatValToStringDecimal (float floatVal)
{
    return { floatVal, NumDecimalPlaces, false };
}
float stringToFloatVal (const juce::String& s);
} // namespace chowdsp::ParamUtils
