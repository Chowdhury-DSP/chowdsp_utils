#pragma once

#include <chowdsp_parameters/chowdsp_parameters.h>

#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
#include <chowdsp_plugin_state/chowdsp_plugin_state.h>

namespace chowdsp::EQ
{
/** Static class for working with standard EQ parameters */
template <size_t NumBands>
struct StandardEQParameters : ParamHolder
{
public:
    static constexpr auto EQNumBands = NumBands;

    /** Parameters to determine the EQ behaviour */
    using Params = BasicEQParams<NumBands>;

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
        const juce::String bandParamPrefix;

        /** Default name prefix for EQ parameters. */
        const juce::String bandNamePrefix = "Band ";

        /** Default version hint for EQ parameters. */
        const int versionHint = 100;

        /** A comma-separated string containing EQ band type choices */
        const juce::StringArray bandTypeChoices = { "1-Pole HPF", "2-Pole HPF", "Low-Shelf", "Bell", "Notch", "High-Shelf", "1-Pole LPF", "2-Pole LPF" };

        /** The index of the default EQ band */
        const int defaultEQBandTypeChoice = 3;

        /** Default frequency for this band */
        const float freqDefault = 1000.0f;

        /** The range to use for the frequency parameter in this band */
        const juce::NormalisableRange<float> freqRange = ParamUtils::createNormalisableRange (20.0f, 20000.0f, 2000.0f);

        /** Default Q-value for this band */
        const float qDefault = CoefficientCalculators::butterworthQ<float>;

        /** The range to use for the Q parameter in this band */
        const juce::NormalisableRange<float> qRange = ParamUtils::createNormalisableRange (0.1f, 20.0f, CoefficientCalculators::butterworthQ<float>);

        /** The range to use for the Gain [dB] parameter in this band */
        const juce::NormalisableRange<float> gainRange = juce::NormalisableRange { -18.0f, 18.0f };

        BoolParameter::Ptr onOffParam {
            juce::ParameterID { bandParamPrefix + "eq_band_on_off", versionHint },
            bandNamePrefix + juce::String (bandIndex + 1) + " On/Off",
            false
        };

        ChoiceParameter::Ptr typeParam {
            juce::ParameterID { bandParamPrefix + "eq_band_type", versionHint },
            bandNamePrefix + juce::String (bandIndex + 1) + " Type",
            bandTypeChoices,
            defaultEQBandTypeChoice
        };

        FreqHzParameter::Ptr freqParam {
            juce::ParameterID { bandParamPrefix + "eq_band_freq", versionHint },
            bandNamePrefix + juce::String (bandIndex + 1) + " Frequency",
            freqRange,
            freqDefault
        };

        FloatParameter::Ptr qParam {
            juce::ParameterID { bandParamPrefix + "eq_band_q", versionHint },
            bandNamePrefix + juce::String (bandIndex + 1) + " Q",
            qRange,
            qDefault,
            &ParamUtils::floatValToString,
            &ParamUtils::stringToFloatVal
        };

        GainDBParameter::Ptr gainParam {
            juce::ParameterID { bandParamPrefix + "eq_band_gain", versionHint },
            bandNamePrefix + juce::String (bandIndex + 1) + " Gain",
            gainRange,
            0.0f
        };

        /** Internal use only! */
        chowdsp::ParamHolder paramHolder { bandNamePrefix + juce::String (bandIndex + 1) };
    };

    /** Set of parameter handles for the entire EQ. */
    using EQParameterHandles = std::array<EQBandParams, NumBands>;

    /** Constructor */
    explicit StandardEQParameters (EQParameterHandles&& paramHandles, const juce::String& name = {});

    /** Parameter handles */
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
