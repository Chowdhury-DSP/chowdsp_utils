#pragma once

#include <chowdsp_parameters/chowdsp_parameters.h>

#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
#include <chowdsp_plugin_state/chowdsp_plugin_state.h>

namespace chowdsp::EQ
{
/** Static class for working with standard EQ parameters */
template <size_t NumBands>
struct StandardEQParameters
{
public:
    static constexpr auto EQNumBands = NumBands;

    /** Parameters to determine the EQ behaviour */
    using Params = BasicEQParams<NumBands>;

    const std::string_view name = "chowdsp_eq";

    struct EQBandParams
    {
        /** The index of this band in the EQ */
        const int bandIndex = 0;

        /**
         * Default parameter ID prefix for EQ parameters.
         *
         * If your plugin contains multiple EQs you may want to
         * use a custom parameter ID prefix to avoid overlap.
         */
        const std::string_view bandParamPrefix;

        /** Default name prefix for EQ parameters. */
        const std::string_view bandNamePrefix = "Band ";

        /**
         * Default version hint for EQ parameters.
         *
         * If your plugin adds an EQ in a version after 1.0.0,
         * you may want to override this value with a custom
         * version hint.
         */
        const int versionHint = 100;

        const float defaultFreqHz = 1000.0f;

        /** A comma-separated string containing EQ band type choices */
        const std::string_view bandTypeChoices = "1-Pole HPF,2-Pole HPF,Low-Shelf,Bell,Notch,High-Shelf,1-Pole LPF,2-Pole LPF";

        /** The index of the default EQ band */
        const int defaultEQBandTypeChoice = 3;

        /** The range to use for the Q parameter in this band */
        const float qMin = 0.1f;
        const float qMax = 10.0f;
        const float qDefault = CoefficientCalculators::butterworthQ<float>;

        BoolParameter::Ptr onOffParam {
            juce::ParameterID { toString (bandParamPrefix) + "eq_band_on_off", versionHint },
            toString (bandNamePrefix) + juce::String (bandIndex + 1) + " On/Off",
            false
        };

        ChoiceParameter::Ptr typeParam {
            juce::ParameterID { toString (bandParamPrefix) + "eq_band_type", versionHint },
            toString (bandNamePrefix) + juce::String (bandIndex + 1) + " Type",
            juce::StringArray::fromTokens (toString (bandTypeChoices), ",", ""),
            defaultEQBandTypeChoice
        };

        FreqHzParameter::Ptr freqParam {
            juce::ParameterID { toString (bandParamPrefix) + "eq_band_freq", versionHint },
            toString (bandNamePrefix) + juce::String (bandIndex + 1) + " Frequency",
            ParamUtils::createNormalisableRange (20.0f, 20000.0f, 2000.0f),
            defaultFreqHz
        };

        FloatParameter::Ptr qParam {
            juce::ParameterID { toString (bandParamPrefix) + "eq_band_q", versionHint },
            toString (bandNamePrefix) + juce::String (bandIndex + 1) + " Q",
            ParamUtils::createNormalisableRange (qMin, qMax, qDefault),
            0.7071f,
            &ParamUtils::floatValToString,
            &ParamUtils::stringToFloatVal
        };

        FreqHzParameter::Ptr gainParam {
            juce::ParameterID { toString (bandParamPrefix) + "eq_band_gain", versionHint },
            toString (bandNamePrefix) + juce::String (bandIndex + 1) + " Gain",
            juce::NormalisableRange { -18.0f, 18.0f },
            0.0f
        };
    };

    /** Set of parameter handles for the entire EQ. */
    using EQParameterHandles = std::array<EQBandParams, NumBands>;

    EQParameterHandles eqParams;

    /** Returns a struct of EQ parameters based on the given set of parameter handles. */
    static Params getEQParameters (const EQParameterHandles& paramHandles);

    /** Sets the parameters of a chowdsp::EQ::EQProcessor, using the given parameters struct. */
    template <typename EQType>
    static void setEQParameters (EQType& eq, const Params& params);

    /** Loads a set of EQ parameters into the StandardEQParameters, from a given parameters struct. */
    static void loadEQParameters (const Params& eqParams, StandardEQParameters& stateParams);
};
} // namespace chowdsp::EQ
#else
namespace chowdsp::EQ
{
/** Static class for working with standard EQ parameters */
template <size_t NumBands>
class StandardEQParameters
{
public:
    static constexpr auto EQNumBands = NumBands;

    /** Parameters to determine the EQ behaviour */
    using Params = BasicEQParams<NumBands>;

    /** Group of parameter handles for a single band. */
    using EQBandParameterHandles = std::tuple<FloatParameter*, FloatParameter*, FloatParameter*, ChoiceParameter*, BoolParameter*>;

    /** Set of parameter handles for the entire EQ. */
    using EQParameterHandles = std::array<EQBandParameterHandles, NumBands>;

    /**
     * Default parameter ID prefix for EQ parameters.
     *
     * If your plugin contains multiple EQs you may want to
     * use a custom parameter ID prefix to avoid overlap.
     */
    inline static const juce::String defaultEQParamPrefix = "chowdsp_eq_";

    /** Initialises a set of EQParameterHandles from a given AudioProcessorValueTreeState. */
    static void initialiseEQParameters (juce::AudioProcessorValueTreeState& vts, EQParameterHandles& params, const juce::String& paramPrefix = defaultEQParamPrefix);

    /**
     * Adds a set of EQParameters to the list of parameters.
     *
     * By default, the "Band Type" parameter will have choices corresponding to
     * chowdsp::EQ::DefaultEQBand, with the "Bell" filter as the default choice.
     * If your EQ uses an EQBand that is different from DefaultEQBandType,
     * custom arguments for eqBandTypeChoices and defaultEQBandTypeChoice
     * must be provided.
     */
    static void addEQParameters (Parameters& params,
                                 const juce::String& paramPrefix = defaultEQParamPrefix,
                                 juce::StringArray eqBandTypeChoices = {},
                                 int defaultEQBandTypeChoice = -1,
                                 int versionHint = 100);

    /** Returns a struct of EQ parameters based on the given set of parameter handles. */
    static Params getEQParameters (const EQParameterHandles& paramHandles);

    /** Sets the parameters of a chowdsp::EQ::EQProcessor, using the given parameters struct. */
    template <typename EQType>
    static void setEQParameters (EQType& eq, const Params& params);

    /** Loads a set of EQ parameters into the AudioProcessorValueTreeState, from a given parameters struct. */
    static void loadEQParameters (const Params& eqParams, juce::AudioProcessorValueTreeState& vts, const juce::String& paramPrefix = defaultEQParamPrefix);

private:
    /** Types of parameters used by each EQ band. */
    enum ParameterType
    {
        FREQ = 0,
        Q,
        GAIN,
        TYPE,
        ONOFF,
    };

    StandardEQParameters() = default; // static use only!
};
} // namespace chowdsp::EQ
#endif

#include "chowdsp_StandardEQParameters.cpp"
