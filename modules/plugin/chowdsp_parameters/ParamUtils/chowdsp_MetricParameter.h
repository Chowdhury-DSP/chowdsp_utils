#pragma once

namespace chowdsp
{
/** A parameter that represents a Metric system value. */
class MetricParameter : public FloatParameter
{
public:
    MetricParameter (const ParameterID& parameterID,
                     const juce::String& paramName,
                     const juce::NormalisableRange<float>& paramRange,
                     float defaultValue,
                     const juce::String& unitSuffix = {},
                     int numDecimalPlaces = 2)
        : FloatParameter (
              parameterID,
              paramName,
              paramRange,
              defaultValue,
              [numDecimalPlaces, unitSuffix] (float v)
              { return toString (v, numDecimalPlaces) + unitSuffix; },
              [] (const juce::String& str)
              { return fromString (str); })
    {
    }

private:
    static juce::String toString (float value, int numDecimalPlaces);
    static float fromString (const juce::String& str);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetricParameter)
};
} // namespace chowdsp
