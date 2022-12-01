#pragma once

#if JUCE_MODULE_AVAILABLE_chowdsp_clap_extensions
#include <chowdsp_clap_extensions/chowdsp_clap_extensions.h>
#endif

namespace chowdsp
{
#if ! JUCE_MODULE_AVAILABLE_chowdsp_clap_extensions
namespace ParamUtils
{
    /** Mixin for parameters that recognize some form of modulation. */
    struct ModParameterMixin
    {
        virtual ~ModParameterMixin() = default;

        /** Returns true if this parameter supports monophonic modulation. */
        virtual bool supportsMonophonicModulation() { return false; }

        /** Returns true if this parameter supports polyphonic modulation. */
        virtual bool supportsPolyphonicModulation() { return false; }

        /** Base function for applying monophonic modulation to a parameter. */
        [[maybe_unused]] virtual void applyMonophonicModulation (double /*value*/)
        {
        }

        /** Base function for applying polyphonic modulation to a parameter. */
        [[maybe_unused]] virtual void applyPolyphonicModulation (int32_t /*note_id*/, int16_t /*port_index*/, int16_t /*channel*/, int16_t /*key*/, double /*value*/)
        {
        }
    };
} // namespace ParamUtils
#endif

#ifndef DOXYGEN
#if JUCE_VERSION < 0x070000
// JUCE 6 doesn't have the new juce::ParameterID class, so this is a little workaround.
using ParameterID = juce::String;
#else
using ParameterID = juce::ParameterID;
#endif
#endif

/** Wrapper of juce::AudioParameterFloat that supports monophonic modulation. */
class FloatParameter : public juce::AudioParameterFloat,
                       public ParamUtils::ModParameterMixin
{
public:
    FloatParameter (const ParameterID& parameterID,
                    const juce::String& parameterName,
                    const juce::NormalisableRange<float>& valueRange,
                    float defaultValue,
                    const std::function<juce::String (float)>& valueToTextFunction,
                    std::function<float (const juce::String&)>&& textToValueFunction);

    using Ptr = OptionalPointer<FloatParameter>;

    /** Returns the default value for the parameter. */
    float getDefaultValue() const override { return unsnappedDefault; }

    /** TRUE! */
    bool supportsMonophonicModulation() override { return true; }

    /** Applies monphonic modulation to this parameter. */
    void applyMonophonicModulation (double value) override;

    /** Returns the current parameter value accounting for any modulation that is currently applied. */
    float getCurrentValue() const noexcept;

    /** Returns the current parameter value accounting for any modulation that is currently applied. */
    operator float() const noexcept { return getCurrentValue(); } // NOSONAR, NOLINT(google-explicit-constructor): we want to be able to do implicit conversion here

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
    ChoiceParameter (const ParameterID& parameterID, const juce::String& parameterName, const juce::StringArray& parameterChoices, int defaultItemIndex)
        : juce::AudioParameterChoice (parameterID, parameterName, parameterChoices, defaultItemIndex)
    {
    }

    using Ptr = OptionalPointer<ChoiceParameter>;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChoiceParameter)
};

/**
 * A Choice parameter based off of an enum class type.
 *
 * By default, underscores in enum names will be replaced with spaces.
 * For custom behaviour, replace the charMap argument with a custom
 * character map.
 */
template <typename EnumType>
class EnumChoiceParameter : public ChoiceParameter
{
public:
    EnumChoiceParameter (const ParameterID& parameterID,
                         const juce::String& parameterName,
                         EnumType defaultChoice,
                         const std::initializer_list<std::pair<char, char>>& charMap = { { '_', ' ' } })
        : ChoiceParameter (
            parameterID,
            parameterName,
            EnumHelpers::createStringArray<EnumType> (charMap),
            static_cast<int> (defaultChoice))
    {
    }

    EnumType get() const noexcept
    {
        return static_cast<EnumType> (getIndex());
    }

    using Ptr = OptionalPointer<EnumChoiceParameter>;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnumChoiceParameter)
};

/** Wrapper of juce::AudioParameterBool that does not support modulation. */
class BoolParameter : public juce::AudioParameterBool,
                      public ParamUtils::ModParameterMixin
{
public:
    BoolParameter (const ParameterID& parameterID, const juce::String& parameterName, bool defaultBoolValue)
        : juce::AudioParameterBool (parameterID, parameterName, defaultBoolValue)
    {
    }

    using Ptr = OptionalPointer<BoolParameter>;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BoolParameter)
};

class PercentParameter : public FloatParameter
{
public:
    PercentParameter (const ParameterID& parameterID,
                      const juce::String& paramName,
                      float defaultValue = 0.5f,
                      bool isBipolar = false)
        : FloatParameter (parameterID,
                          paramName,
                          juce::NormalisableRange { isBipolar ? -1.0f : 0.0f, 1.0f },
                          defaultValue,
                          &ParamUtils::percentValToString,
                          &ParamUtils::stringToPercentVal)
    {
    }

    using Ptr = OptionalPointer<PercentParameter>;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PercentParameter)
};

class GainDBParameter : public FloatParameter
{
public:
    GainDBParameter (const ParameterID& parameterID,
                     const juce::String& paramName,
                     const juce::NormalisableRange<float>& paramRange,
                     float defaultValue)
        : FloatParameter (parameterID,
                          paramName,
                          paramRange,
                          defaultValue,
                          &ParamUtils::gainValToString,
                          &ParamUtils::stringToGainVal)
    {
    }

    using Ptr = OptionalPointer<GainDBParameter>;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainDBParameter)
};

class FreqHzParameter : public FloatParameter
{
public:
    FreqHzParameter (const ParameterID& parameterID,
                     const juce::String& paramName,
                     const juce::NormalisableRange<float>& paramRange,
                     float defaultValue)
        : FloatParameter (parameterID,
                          paramName,
                          paramRange,
                          defaultValue,
                          &ParamUtils::freqValToString,
                          &ParamUtils::stringToFreqVal)
    {
    }

    using Ptr = OptionalPointer<FreqHzParameter>;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FreqHzParameter)
};
} // namespace chowdsp
