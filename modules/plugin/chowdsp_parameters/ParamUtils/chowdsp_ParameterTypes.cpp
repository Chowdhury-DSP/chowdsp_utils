#include "chowdsp_ParameterTypes.h"

namespace chowdsp
{
FloatParameter::FloatParameter (const juce::String& parameterID,
                                const juce::String& parameterName,
                                const juce::NormalisableRange<float>& valueRange,
                                float defaultFloatValue,
                                const std::function<juce::String (float)>& valueToTextFunction,
                                std::function<float (const juce::String&)>&& textToValueFunction) : juce::AudioParameterFloat (
    parameterID,
    parameterName,
    valueRange,
    defaultFloatValue,
    juce::String(),
    AudioProcessorParameter::genericParameter,
    valueToTextFunction == nullptr ? std::function<juce::String (float v, int)>()
                                   : [valueToTextFunction] (float v, int) { return valueToTextFunction (v); },
    std::move (textToValueFunction)),
                                                                                                    unsnappedDefault (valueRange.convertTo0to1 (defaultFloatValue)),
                                                                                                    normalisableRange (valueRange)
{
}

void FloatParameter::applyMonophonicModulation (double modulationValue)
{
    modulationAmount = (float) modulationValue;
}

float FloatParameter::getCurrentValue() const noexcept
{
    return normalisableRange.convertFrom0to1 (juce::jlimit (0.0f, 1.0f, normalisableRange.convertTo0to1 (get()) + modulationAmount));
}
} // namespace chowdsp
