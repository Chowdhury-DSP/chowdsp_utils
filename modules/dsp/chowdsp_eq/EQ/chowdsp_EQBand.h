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
template <typename FloatType, typename FilterChoicesTuple>
class EQBandBase
{
public:
    using NumericType = SampleTypeHelpers::NumericType<FloatType>;

    /** Default constructor */
    EQBandBase();

    /** Sets the cutoff frequency of the EQ band in Hz */
    void setCutoffFrequency (NumericType newCutoffHz);

    /**
     * Sets the Q value of the EQ band.
     * If the current filter type does not have a Q parameter,
     * this method will have no apparent effect.
     */
    void setQValue (NumericType newQValue);

    /**
     * Sets the linear gain of the EQ band.
     * If the current filter type does not have a gain parameter,
     * this method will have no apparent effect.
     */
    void setGain (NumericType newGain);

    /**
     * Sets the gain of the EQ band in Decibels.
     * If the current filter type does not have a gain parameter,
     * this method will have no apparent effect.
     */
    void setGainDB (NumericType newGainDB);

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

    /** Processes an buffer of samples. */
    void processBlock (const BufferView<FloatType>& buffer, ArenaAllocatorView arena) noexcept;

private:
    template <typename FilterType, typename T = FloatType, size_t N = FilterType::Order>
    std::enable_if_t<std::is_base_of_v<IIRFilter<N, T>, FilterType> || std::is_base_of_v<SOSFilter<N, T>, FilterType> || std::is_base_of_v<SOSFilter<N - 1, T>, FilterType>, void>
        processFilterChannel (FilterType& filter, const BufferView<FloatType>& block);

    template <typename FilterType, typename T = FloatType, StateVariableFilterType type = FilterType::Type>
    std::enable_if_t<std::is_same_v<StateVariableFilter<T, type>, FilterType>, void>
        processFilterChannel (FilterType& filter, const BufferView<FloatType>& block);

    template <typename FilterType, typename T = FloatType, size_t N = FilterType::Order, StateVariableFilterType type = FilterType::Type>
    std::enable_if_t<std::is_same_v<NthOrderFilter<T, N, type>, FilterType>, void>
        processFilterChannel (FilterType& filter, const BufferView<FloatType>& block);

    void fadeBuffers (const FloatType* fadeInBuffer, const FloatType* fadeOutBuffer, FloatType* targetBuffer, int numSamples) const;

    static constexpr auto numFilterChoices = std::tuple_size<FilterChoicesTuple>();
    FilterChoicesTuple filters;

    NumericType freqHzHandle = 1000.0f;
    NumericType qHandle = 0.7071f;
    NumericType gainHandle = 1.0f;

    SmoothedBufferValue<NumericType, juce::ValueSmoothingTypes::Multiplicative> freqSmooth;
    SmoothedBufferValue<NumericType, juce::ValueSmoothingTypes::Multiplicative> qSmooth;
    SmoothedBufferValue<NumericType, juce::ValueSmoothingTypes::Multiplicative> gainSmooth;
    int filterType = 0, prevFilterType = 0;

    NumericType fs = NumericType (44100.0);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQBandBase)
};

template <typename FloatType, typename... FilterChoices>
struct EQBand : EQBandBase<FloatType, std::tuple<FilterChoices...>>
{
    using FilterChoicesTuple = std::tuple<FilterChoices...>;
};

template <typename FloatType = float>
using DefaultEQBand = EQBand<FloatType,
                             FirstOrderHPF<FloatType>,
                             SVFHighpass<FloatType>,
                             SVFLowShelf<FloatType>,
                             SVFBell<FloatType>,
                             SVFNotch<FloatType>,
                             SVFHighShelf<FloatType>,
                             FirstOrderLPF<FloatType>,
                             SVFLowpass<FloatType>>;

} // namespace chowdsp::EQ

#include "chowdsp_EQBand.cpp"
