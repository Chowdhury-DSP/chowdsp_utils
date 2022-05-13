#pragma once

namespace chowdsp::EQ
{
/**
 * A fully functioning multi-band equaliser!
 *
 * The EQBandType should be a chowdsp::EQBand.
 */
template <typename FloatType, size_t numBands, typename EQBandType>
class EQProcessor
{
public:
    /** Default constructor */
    EQProcessor();

    /** Sets the cutoff frequency of an EQ band in Hz */
    void setCutoffFrequency (int band, FloatType newCutoffHz);

    /**
     * Sets the Q value of an EQ band.
     * If the band's current filter type does not have a Q parameter,
     * this method will have no apparent effect.
     */
    void setQValue (int band, FloatType newQValue);

    /**
     * Sets the linear gain of an EQ band.
     * If the band's current filter type does not have a gain parameter,
     * this method will have no apparent effect.
     */
    void setGain (int band, FloatType newGain);

    /**
     * Sets the gain of an EQ band in Decibels.
     * If the band's current filter type does not have a gain parameter,
     * this method will have no apparent effect.
     */
    void setGainDB (int band, FloatType newGainDB);

    /**
     * Selects the type of filter to use for an EQ band.
     * The filter type must be an index into the list of FilterChoices
     * provided as the EQBand template parameter.
     */
    void setFilterType (int band, int newFilterType);

    /** Turns a single EQ band on or off. */
    void setBandOnOff (int band, bool shouldBeOn);

    /** Prepares the EQ to process a new stream of audio */
    void prepare (const juce::dsp::ProcessSpec& spec);

    /** Resets the EQ state */
    void reset();

    /** Processes an audio block */
    void process (chowdsp::AudioBlock<FloatType> block);

private:
    std::array<EQBandType, numBands> bands;
    std::array<BypassProcessor<FloatType>, numBands> bypasses;
    std::array<bool, numBands> onOffs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQProcessor)
};
} // namespace chowdsp::EQ

#include "chowdsp_EQProcessor.cpp"
