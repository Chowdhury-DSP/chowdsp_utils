#pragma once

#include <tuple>

namespace chowdsp::EQ
{
/**
 * A set of filters intended to be used as a single "band" in a multiband EQ.
 *
 * Provide the types of filters that the band may use to the FilterChoices
 * template parameter. Currently only the filter types derived from
 * chowdsp::IIRFilter<> are supported.
 */
template <typename FloatType, typename... FilterChoices>
class EQBand
{
public:
    /** Default constructor */
    EQBand();

    /** Sets the cutoff frequency of the EQ band in Hz */
    void setCutoffFrequency (FloatType newCutoffHz);

    /**
     * Sets the Q value of the EQ band.
     * If the current filter type does not have a Q parameter,
     * this method will have no apparent effect.
     */
    void setQValue (FloatType newQValue);

    /**
     * Sets the linear gain of the EQ band.
     * If the current filter type does not have a gain parameter,
     * this method will have no apparent effect.
     */
    void setGain (FloatType newGain);

    /**
     * Sets the gain of the EQ band in Decibels.
     * If the current filter type does not have a gain parameter,
     * this method will have no apparent effect.
     */
    void setGainDB (FloatType newGainDB);

    /**
     * Selects the type of filter to use for the EQ band.
     * The filter type must be an index into the list of FilterChoices
     * provided as the class template parameter.
     */
    void setFilterType (int newFilterType);

    /** Prepares the EQBand to process a new stream of audio */
    void prepare (const juce::dsp::ProcessSpec& spec);

    /** Resets the EQ band state */
    void reset();

    /** Processes an audio context */
    template <typename ProcessContext>
    void process (const ProcessContext& context);

private:
    template <typename FilterType, typename T = FloatType, size_t N = FilterType::Order>
    std::enable_if_t<std::is_base_of_v<IIRFilter<N, T>, FilterType> || std::is_base_of_v<SOSFilter<N, T>, FilterType>, void>
        processFilterChannel (FilterType& filter, chowdsp::AudioBlock<FloatType>& block);

    template <typename FilterType, typename T = FloatType, size_t N = FilterType::Order, StateVariableFilterType type = FilterType::Type>
    std::enable_if_t<std::is_base_of_v<NthOrderFilter<T, N, type>, FilterType>, void>
        processFilterChannel (FilterType& filter, chowdsp::AudioBlock<FloatType>& block);

    void fadeBuffers (const FloatType* fadeInBuffer, const FloatType* fadeOutBuffer, FloatType* targetBuffer, int numSamples);

    static constexpr auto numFilterChoices = sizeof...(FilterChoices);
    using Filters = std::tuple<FilterChoices...>;
    Filters filters;

    FloatType freqHzHandle = 1000.0f;
    FloatType qHandle = 0.7071f;
    FloatType gainHandle = 1.0f;

    SmoothedBufferValue<FloatType, juce::ValueSmoothingTypes::Multiplicative> freqSmooth;
    SmoothedBufferValue<FloatType, juce::ValueSmoothingTypes::Multiplicative> qSmooth;
    SmoothedBufferValue<FloatType, juce::ValueSmoothingTypes::Multiplicative> gainSmooth;
    int filterType = 0, prevFilterType = 0;

    FloatType fs = FloatType (44100.0);

    juce::AudioBuffer<FloatType> fadeBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQBand)
};

template <typename FloatType>
using DefaultEQBand = EQBand<FloatType,
                             FirstOrderHPF<FloatType>,
                             SecondOrderHPF<FloatType>,
                             LowShelfFilter<FloatType>,
                             PeakingFilter<FloatType>,
                             NotchFilter<FloatType>,
                             HighShelfFilter<FloatType>,
                             FirstOrderLPF<FloatType>,
                             SecondOrderLPF<FloatType>>;

} // namespace chowdsp::EQ

#include "chowdsp_EQBand.cpp"
