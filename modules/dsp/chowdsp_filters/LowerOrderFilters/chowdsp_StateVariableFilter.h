#pragma once

namespace chowdsp
{
/** Filter type options for State Variable Filters */
enum class StateVariableFilterType
{
    Lowpass,
    Bandpass,
    Highpass,
    Notch,
    Allpass,
    Bell,
    LowShelf,
    HighShelf,
    MultiMode, /**< Allows the filter to be interpolated between lowpass, bandpass, and highpass */
    Crossover, /**< Returns both the highpass and lowpass outputs of the filter, for use in crossover filters */
};

/**
 * A State Variable Filter, as derived by Andy Simper (Cytomic).
 *
 * Reference: https://cytomic.com/files/dsp/SvfLinearTrapAllOutputs.pdf
 */
template <typename SampleType, StateVariableFilterType type, size_t maxChannelCount = defaultChannelCount>
class StateVariableFilter
{
public:
    static constexpr int Order = 2;
    static constexpr auto Type = type;

    using FilterType = StateVariableFilterType;
    using NumericType = SampleTypeHelpers::NumericType<SampleType>;

    /** Constructor. */
    StateVariableFilter();

    /**
     * Sets the cutoff frequency of the filter.
     *
     * @param newFrequencyHz the new cutoff frequency in Hz.
    */
    template <bool shouldUpdate = true>
    void setCutoffFrequency (SampleType newFrequencyHz);

    /**
     * Sets the resonance of the filter.
     *
     * Note: The bandwidth of the resonance increases with the value of the
     * parameter. To have a standard 12 dB / octave filter, the value must be set
     * at 1 / sqrt(2).
    */
    template <bool shouldUpdate = true>
    void setQValue (SampleType newResonance);

    /**
     * Sets the gain of the filter in units of linear gain.
     *
     * Note that for some filter types (Lowpass, Highpass, Bandpass, Allpass)
     * this control will have no effect.
     */
    template <bool shouldUpdate = true>
    void setGain (SampleType newGainLinear);

    /**
     * Sets the gain of the filter in units of Decibels.
     *
     * Note that for some filter types (Lowpass, Highpass, Bandpass, Allpass)
     * this control will have no effect.
     */
    template <bool shouldUpdate = true>
    void setGainDecibels (SampleType newGainDecibels);

    /**
     * Sets the Mode of a multi-mode filter. The mode parameter is expected to be in [0, 1],
     * where 0 corresponds to a LPF, 0.5 corresponds to a BPF, and 1 corresponds to a HPF.
     */
    template <StateVariableFilterType M = type>
    std::enable_if_t<M == StateVariableFilterType::MultiMode, void> setMode (NumericType mode);

    /**
     * Updates the filter coefficients.
     *
     * Don't touch this unless you know what you're doing!
     */
    void update();

    /** Returns the cutoff frequency of the filter. */
    [[nodiscard]] SampleType getCutoffFrequency() const noexcept { return cutoffFrequency; }

    /** Returns the resonance of the filter. */
    [[nodiscard]] SampleType getQValue() const noexcept { return resonance; }

    /** Returns the gain of the filter. */
    [[nodiscard]] SampleType getGain() const noexcept { return gain; }

    /** Initialises the filter. */
    void prepare (const juce::dsp::ProcessSpec& spec);

    /** Resets the internal state variables of the filter. */
    void reset();

    /**
     * Ensure that the state variables are rounded to zero if the state
     * variables are denormals. This is only needed if you are doing
     * sample by sample processing.
    */
    void snapToZero() noexcept;

    /** Process block of samples */
    template <StateVariableFilterType M = type>
    std::enable_if_t<M != StateVariableFilterType::Crossover, void> processBlock (const BufferView<SampleType>& block) noexcept
    {
        for (auto [channel, sampleData] : buffer_iters::channels (block))
        {
            ScopedValue s1 { ic1eq[(size_t) channel] };
            ScopedValue s2 { ic2eq[(size_t) channel] };

            for (auto& sample : sampleData)
                sample = processSampleInternal (sample, s1.get(), s2.get());
        }

#if JUCE_SNAP_TO_ZERO
        snapToZero();
#endif
    }

    /** Process block of samples */
    template <StateVariableFilterType M = type>
    std::enable_if_t<M == StateVariableFilterType::Crossover, void> processBlock (const BufferView<const SampleType>& blockIn,
                                                                                  const BufferView<SampleType>& blockLow,
                                                                                  const BufferView<SampleType>& blockHigh) noexcept
    {
        const auto numChannels = blockIn.getNumChannels();
        const auto numSamples = blockIn.getNumSamples();

        jassert (blockLow.getNumChannels() == numChannels);
        jassert (blockHigh.getNumChannels() == numChannels);
        jassert (blockLow.getNumSamples() == numSamples);
        jassert (blockHigh.getNumSamples() == numSamples);

        for (int channel = 0; channel < numChannels; ++channel)
        {
            const auto* inData = blockIn.getReadPointer (channel);
            auto* outDataLow = blockLow.getWritePointer (channel);
            auto* outDataHigh = blockHigh.getWritePointer (channel);
            ScopedValue s1 { ic1eq[(size_t) channel] };
            ScopedValue s2 { ic2eq[(size_t) channel] };

            for (int i = 0; i < numSamples; ++i)
                std::tie (outDataLow[i], outDataHigh[i]) = processSampleInternal (inData[i], s1.get(), s2.get());
        }

#if JUCE_SNAP_TO_ZERO
        snapToZero();
#endif
    }

    /** Processes the input and output samples supplied in the processing context. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples = outputBlock.getNumSamples();

        jassert (inputBlock.getNumChannels() <= ic1eq.size());
        jassert (inputBlock.getNumChannels() == numChannels);
        jassert (inputBlock.getNumSamples() == numSamples);

        if (context.isBypassed)
        {
            outputBlock.copyFrom (inputBlock);
            return;
        }

        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            auto* inputSamples = inputBlock.getChannelPointer (channel);
            auto* outputSamples = outputBlock.getChannelPointer (channel);

            ScopedValue s1 { ic1eq[channel] };
            ScopedValue s2 { ic2eq[channel] };

            for (size_t i = 0; i < numSamples; ++i)
                outputSamples[i] = processSampleInternal (inputSamples[i], s1.get(), s2.get());
        }

#if JUCE_SNAP_TO_ZERO
        snapToZero();
#endif
    }

    /**
     * Processes one sample at a time on a given channel.
     *
     * In "Crossover" mode this method will return a pair of (low-band, high-band).
     */
    inline auto processSample (int channel, SampleType inputValue) noexcept
    {
        return processSampleInternal (inputValue, ic1eq[(size_t) channel], ic2eq[(size_t) channel]);
    }

    /** Internal use only! */
    inline auto processSampleInternal (SampleType x, SampleType& s1, SampleType& s2) noexcept
    {
        const auto [v0, v1, v2] = processCore (x, s1, s2);

        juce::ignoreUnused (v0);
        if constexpr (type == FilterType::Lowpass)
            return v2;
        else if constexpr (type == FilterType::Bandpass)
            return v1;
        else if constexpr (type == FilterType::Highpass)
            return v0;
        else if constexpr (type == FilterType::Notch)
            return v2 + v0; // low + high
        else if constexpr (type == FilterType::Allpass)
            return v2 + v0 - k0 * v1; // low + high - k * band
        else if constexpr (type == FilterType::Bell)
            return v2 + v0 + k0A * v1; // low + high + k0 * A * band
        else if constexpr (type == FilterType::LowShelf)
            return Asq * v2 + k0A * v1 + v0; // Asq * low + k0 * A * band + high
        else if constexpr (type == FilterType::HighShelf)
            return Asq * v0 + k0A * v1 + v2; // Asq * high + k0 * A * band + low
        else if constexpr (type == FilterType::MultiMode)
            return lowpassMult * v2 + bandpassMult * v1 + highpassMult * v0;
        else if constexpr (type == FilterType::Crossover)
        {
            return std::make_pair (v2, -v0);
        }
        else
        {
            jassertfalse; // unknown filter type!
            return SampleType {};
        }
    }

    /** Internal use only! */
    inline auto processCore (SampleType x, SampleType& s1, SampleType& s2) noexcept
    {
        const auto v3 = x - s2;
        const auto v0 = a1 * v3 - ak * s1;
        const auto v1 = a2 * v3 + a1 * s1;
        const auto v2 = a3 * v3 + a2 * s1 + s2;

        // update state
        s1 = (NumericType) 2 * v1 - s1;
        s2 = (NumericType) 2 * v2 - s2;

        return std::make_tuple (v0, v1, v2);
    }

    using State = std::conditional_t<maxChannelCount == dynamicChannelCount, std::vector<SampleType>, std::array<SampleType, maxChannelCount>>;
    State ic1eq {}, ic2eq {}; // state variables

private:
    SampleType cutoffFrequency, resonance, gain; // parameters
    SampleType g0, k0, A, sqrtA; // parameter intermediate values
    SampleType a1, a2, a3, ak, k0A, Asq; // coefficients

    NumericType lowpassMult { 0 }, bandpassMult { 0 }, highpassMult { 0 };

    double sampleRate = 44100.0;

    template <typename>
    friend class ARPFilter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StateVariableFilter)
};

/** Convenient alias for an SVF lowpass filter. */
template <typename SampleType = float, size_t maxChannelCount = defaultChannelCount>
using SVFLowpass = StateVariableFilter<SampleType, StateVariableFilterType::Lowpass, maxChannelCount>;

/** Convenient alias for an SVF highpass filter. */
template <typename SampleType = float, size_t maxChannelCount = defaultChannelCount>
using SVFHighpass = StateVariableFilter<SampleType, StateVariableFilterType::Highpass, maxChannelCount>;

/** Convenient alias for an SVF bandpass filter. */
template <typename SampleType = float, size_t maxChannelCount = defaultChannelCount>
using SVFBandpass = StateVariableFilter<SampleType, StateVariableFilterType::Bandpass, maxChannelCount>;

/** Convenient alias for an SVF allpass filter. */
template <typename SampleType = float, size_t maxChannelCount = defaultChannelCount>
using SVFAllpass = StateVariableFilter<SampleType, StateVariableFilterType::Allpass, maxChannelCount>;

/** Convenient alias for an SVF notch filter. */
template <typename SampleType = float, size_t maxChannelCount = defaultChannelCount>
using SVFNotch = StateVariableFilter<SampleType, StateVariableFilterType::Notch, maxChannelCount>;

/** Convenient alias for an SVF bell filter. */
template <typename SampleType = float, size_t maxChannelCount = defaultChannelCount>
using SVFBell = StateVariableFilter<SampleType, StateVariableFilterType::Bell, maxChannelCount>;

/** Convenient alias for an SVF low-shelf filter. */
template <typename SampleType = float, size_t maxChannelCount = defaultChannelCount>
using SVFLowShelf = StateVariableFilter<SampleType, StateVariableFilterType::LowShelf, maxChannelCount>;

/** Convenient alias for an SVF high-shelf filter. */
template <typename SampleType = float, size_t maxChannelCount = defaultChannelCount>
using SVFHighShelf = StateVariableFilter<SampleType, StateVariableFilterType::HighShelf, maxChannelCount>;

/** Convenient alias for an SVF multi-mode filter. */
template <typename SampleType = float, size_t maxChannelCount = defaultChannelCount>
using SVFMultiMode = StateVariableFilter<SampleType, StateVariableFilterType::MultiMode, maxChannelCount>;
} // namespace chowdsp

#include "chowdsp_StateVariableFilter.cpp"
