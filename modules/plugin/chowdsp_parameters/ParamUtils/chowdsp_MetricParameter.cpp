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
    const auto absValue = std::abs (value);
    float valueInRange { value };
    juce::String suffix {};

    if (absValue < 1.0e-12f) // femto
    {
        valueInRange = value * 1.0e15f;
        suffix = " f";
    }
    else if (absValue < 1.0e-9f) // pico
    {
        valueInRange = value * 1.0e12f;
        suffix = " p";
    }
    else if (absValue < 1.0e-6f) // nano
    {
        valueInRange = value * 1.0e9f;
        suffix = " n";
    }
    else if (absValue < 1.0e-3f) // micro
    {
        valueInRange = value * 1.0e6f;
        suffix = juce::String::fromUTF8 (" μ");
    }
    else if (absValue < 1.0f) // milli
    {
        valueInRange = value * 1.0e3f;
        suffix = " m";
    }
    else if (absValue < 1.0e3f) // units
    {
        suffix = " ";
    }
    else if (absValue < 1.0e6f) // kilo
    {
        valueInRange = value * 1.0e-3f;
        suffix = " k";
    }
    else if (absValue < 1.0e9f) // mega
    {
        valueInRange = value * 1.0e-6f;
        suffix = " M";
    }
    else // Giga
    {
        valueInRange = value * 1.0e-9f;
        suffix = " G";
    }

    juce::String res;
    if (numDecimalPlaces == 0)
        res = juce::String { static_cast<int> (valueInRange) };
    else
        res = juce::String { valueInRange, numDecimalPlaces };
    res += suffix;

    return res;
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
        if (sfx.containsAnyOf ("f"))
            return 1.0e-15f;
        return 1.0f;
    }(suffix);

    return baseNumber * multiplier;
}
} // namespace chowdsp
