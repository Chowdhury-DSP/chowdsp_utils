#pragma once

#include <chowdsp_parameters/chowdsp_parameters.h>

namespace chowdsp::EQ
{
/** Static class for working with standard EQ parameters */
template <size_t NumBands>
class StandardEQParameters
{
public:
    static constexpr auto EQNumBands = NumBands;

    /** Parameters to determine the EQ behaviour */
    struct Params
    {
        struct BandParams
        {
            float bandFreqHz, bandQ, bandGainDB;
            int bandType;
            bool bandOnOff;
        };

        std::array<chowdsp::EQ::EQParams<BandParams>, NumBands> bands;
    };

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
    static void addEQParameters (Parameters& params, const juce::String& paramPrefix = defaultEQParamPrefix, juce::StringArray eqBandTypeChoices = {}, int defaultEQBandTypeChoice = -1);

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

#include "chowdsp_StandardEQParameters.cpp"
