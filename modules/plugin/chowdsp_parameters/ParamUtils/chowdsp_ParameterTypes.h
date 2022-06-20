#pragma once

#if HAS_CLAP_JUCE_EXTENSIONS
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunused-parameter", "-Wextra-semi")
#include "clap-juce-extensions/clap-juce-extensions.h"
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
#endif

namespace chowdsp
{
namespace ParamUtils
{
    /** Mixin for parameters that recognize some form of modulation. */
    struct ModParameterMixin
#if HAS_CLAP_JUCE_EXTENSIONS
        : public clap_juce_extensions::clap_param_extensions
#endif
    {
        ModParameterMixin() = default;

#if ! HAS_CLAP_JUCE_EXTENSIONS
        virtual ~ModParameterMixin() = default;

        /** Returns true if this parameter supports monophonic modulation. */
        virtual bool supportsMonophonicModulation() { return false; }

        /** Returns true if this parameter supports polyphonic modulation. */
        virtual bool supportsPolyphonicModulation() { return false; }
#endif

        /** Base function for applying monophonic modulation to a parameter. */
        [[maybe_unused]] virtual void applyMonophonicModulation (double /*value*/)
        {
        }

        /** Base function for applying polyphonic modulation to a parameter. */
        [[maybe_unused]] virtual void applyPolyphonicModulation (int32_t /*note_id*/, int16_t /*key*/, int16_t /*channel*/, double /*value*/)
        {
        }
    };
} // namespace ParamUtils

/** Wrapper of juce::AudioParameterFloat that supports monophonic modulation. */
class FloatParameter : public juce::AudioParameterFloat,
                       public ParamUtils::ModParameterMixin
{
public:
    FloatParameter (const juce::String& parameterID,
                    const juce::String& parameterName,
                    const juce::NormalisableRange<float>& valueRange,
                    float defaultValue,
                    const std::function<juce::String (float)>& valueToTextFunction,
                    std::function<float (const juce::String&)>&& textToValueFunction);

    /** Returns the default value for the parameter. */
    float getDefaultValue() const override { return unsnappedDefault; }

    /** TRUE */
    bool supportsMonophonicModulation() override { return true; }

    /** Applies monphonic modulation to this parameter. */
    void applyMonophonicModulation (double value) override;

    /** Returns the current parameter value accounting for any modulation that is currently applied. */
    float getCurrentValue() const noexcept;

    /** Returns the current parameter value accounting for any modulation that is currently applied. */
    operator float() const noexcept { return getCurrentValue(); }

private:
    const float unsnappedDefault;
    const juce::NormalisableRange<float> normalisableRange;

    float modulationAmount = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FloatParameter)
};

/** Wrapper of juce::AudioParameterChoice that does not support modulation. */
class ChoiceParameter : public juce::AudioParameterChoice,
                        public ParamUtils::ModParameterMixin
{
public:
    ChoiceParameter (const juce::String& parameterID, const juce::String& parameterName, const juce::StringArray& parameterChoices, int defaultItemIndex)
        : juce::AudioParameterChoice (parameterID, parameterName, parameterChoices, defaultItemIndex)
    {
    }
};

/** Wrapper of juce::AudioParameterBool that does not support modulation. */
class BoolParameter : public juce::AudioParameterBool,
                      public ParamUtils::ModParameterMixin
{
public:
    BoolParameter (const juce::String& parameterID, const juce::String& parameterName, bool defaultValue)
        : juce::AudioParameterBool (parameterID, parameterName, defaultValue)
    {
    }
};
} // namespace chowdsp
