#include "chowdsp_ParameterTypes.h"

namespace chowdsp
{

FloatParameter::FloatParameter (const juce::String& parameterID,
                                const juce::String& parameterName,
                                const juce::NormalisableRange<float>& valueRange,
                                float defaultValue,
                                const std::function<juce::String (float)>& valueToTextFunction,
                                std::function<float (const juce::String&)>&& textToValueFunction) : juce::AudioParameterFloat (
    parameterID,
    parameterName,
    valueRange,
    defaultValue,
    juce::String(),
    AudioProcessorParameter::genericParameter,
    valueToTextFunction == nullptr ? std::function<juce::String (float v, int)>()
                                   : [valueToTextFunction] (float v, int)
        { return valueToTextFunction (v); },
    std::move (textToValueFunction)),
                                                                                                    unsnappedDefault (valueRange.convertTo0to1 (defaultValue)),
                                                                                                    normalisableRange (valueRange)
{
}

#if HAS_CLAP_JUCE_EXTENSIONS
void FloatParameter::applyMonophonicModulation (double value)
{
    modulationAmount = (float) value;
}

float FloatParameter::getCurrentValue() const noexcept
{
    return normalisableRange.convertFrom0to1 (juce::jlimit (0.0f, 1.0f, normalisableRange.convertTo0to1 (get()) + modulationAmount));
}
#else
void FloatParameter::applyMonophonicModulation (double) {}
float FloatParameter::getCurrentValue() const noexcept { return get(); }
#endif
} // namespace chowdsp
