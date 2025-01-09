#include "chowdsp_MetricParameter.h"

namespace chowdsp
{
MetricParameter::MetricParameter (const ParameterID& parameterID,
                                  const juce::String& paramName,
                                  const juce::NormalisableRange<float>& paramRange,
                                  float defaultValue,
                                  const juce::String& unitSuffix,
                                  int numDecimalPlaces)
    : MetricParameter (
          parameterID,
          paramName,
          paramRange,
          defaultValue,
          [numDecimalPlaces, unitSuffix] (float v)
          { return toString (v, numDecimalPlaces) + unitSuffix; })
{
}

MetricParameter::MetricParameter (
    const ParameterID& parameterID,
    const juce::String& paramName,
    const juce::NormalisableRange<float>& paramRange,
    float defaultValue,
    const std::function<juce::String (float)>& valueToTextFunction,
    std::function<float (const juce::String&)>&& textToValueFunction)
    : FloatParameter (
          parameterID,
          paramName,
          paramRange,
          defaultValue,
          valueToTextFunction,
          textToValueFunction != nullptr
              ? std::move (textToValueFunction)
              : [] (const juce::String& str)
              { return fromString (str); })
{
}

juce::String MetricParameter::toString (float value, int numDecimalPlaces)
{
    if (value < 1.0e-9f) // pico
    {
        return juce::String { value * 1.0e12f, numDecimalPlaces } + " p";
    }
    if (value < 1.0e-6f) // nano
    {
        return juce::String { value * 1.0e9f, numDecimalPlaces } + " n";
    }
    if (value < 1.0e-3f) // micro
    {
        return juce::String { value * 1.0e6f, numDecimalPlaces } + " μ";
    }
    if (value < 1.0f) // milli
    {
        return juce::String { value * 1.0e3f, numDecimalPlaces } + " m";
    }
    if (value < 1.0e3f) // units
    {
        return juce::String { value, numDecimalPlaces } + " ";
    }
    if (value < 1.0e6f) // kilo
    {
        return juce::String { value * 1.0e-3f, numDecimalPlaces } + " k";
    }
    if (value < 1.0e9f) // mega
    {
        return juce::String { value * 1.0e-6f, numDecimalPlaces } + " M";
    }

    // Giga
    return juce::String { value * 1.0e-9f, numDecimalPlaces } + " G";
}

float MetricParameter::fromString (const juce::String& str)
{
    juce::String justTheNumbers {};
    juce::String suffix {};
    for (int i = 0; i < str.length(); ++i)
    {
        const auto indexOfNumber = str.indexOfAnyOf ("0123456789.", i);
        if (indexOfNumber < 0)
        {
            suffix = str.substring (i);
            break;
        }

        justTheNumbers += str[i];
    }

    const auto baseNumber = justTheNumbers.getFloatValue();
    const auto multiplier = [] (const juce::String& sfx) -> float
    {
        if (sfx.containsAnyOf ("G"))
            return 1.0e9f;
        if (sfx.containsAnyOf ("M"))
            return 1.0e6f;
        if (sfx.containsAnyOf ("kK"))
            return 1.0e3f;
        if (sfx.containsAnyOf ("m"))
            return 1.0e-3f;
        if (sfx.containsAnyOf ("uμ"))
            return 1.0e-6f;
        if (sfx.containsAnyOf ("n"))
            return 1.0e-9f;
        if (sfx.containsAnyOf ("p"))
            return 1.0e-12f;
        return 1.0f;
    }(suffix);

    return baseNumber * multiplier;
}
} // namespace chowdsp
