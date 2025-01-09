#pragma once

namespace chowdsp
{
/** A parameter that represents a Metric system value. */
class MetricParameter : public FloatParameter
{
public:
    using Ptr = OptionalPointer<MetricParameter>;

    MetricParameter (const ParameterID& parameterID,
                     const juce::String& paramName,
                     const juce::NormalisableRange<float>& paramRange,
                     float defaultValue,
                     const juce::String& unitSuffix = {},
                     int numDecimalPlaces = 2);

    MetricParameter (
        const ParameterID& parameterID,
        const juce::String& paramName,
        const juce::NormalisableRange<float>& paramRange,
        float defaultValue,
        const std::function<juce::String (float)>& valueToTextFunction,
        std::function<float (const juce::String&)>&& textToValueFunction = nullptr);

    static juce::String toString (float value, int numDecimalPlaces);
    static float fromString (const juce::String& str);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetricParameter)
};
} // namespace chowdsp
