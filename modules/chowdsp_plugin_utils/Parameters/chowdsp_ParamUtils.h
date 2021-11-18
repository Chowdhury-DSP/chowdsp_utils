#pragma once

namespace chowdsp
{
namespace ParamUtils
{
    using VTSParam = juce::AudioProcessorValueTreeState::Parameter;

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

    juce::String floatValToString (float floatVal);
    juce::String floatValToStringDecimal (float floatVal, int numDecimalPlaces);
    float stringToFloatVal (const juce::String& s);

} // namespace ParamUtils

} // namespace chowdsp
