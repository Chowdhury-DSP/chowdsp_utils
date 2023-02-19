#pragma once

namespace chowdsp
{
/** Filter type options for the ARP Filter */
enum class ARPFilterType
{
    Lowpass = 1,
    Bandpass = 2,
    Highpass = 4,
    Notch = 8, /**< The ARP filter notch mode has an extra funky notch offset parameter */
};

/**
 * Emulation of the ARP 1047 Multi-Mode Filter/Resonator.
 * Similar to StateVariableFilter, but with a "limit mode" option,
 * and a "notch offset" feature.
 *
 * Reference: https://www.guitarfool.com/ARP2500/DennisCollinPaper.pdf
 */
template <typename SampleType>
class ARPFilter
{
public:
    using FilterType = ARPFilterType;
    using NumericType = SampleTypeHelpers::NumericType<SampleType>;

    ARPFilter() = default;

    /**
     * Sets the cutoff frequency of the filter.
     *
     * @param newFrequencyHz the new cutoff frequency in Hz.
    */
    template <bool shouldUpdate = true>
    void setCutoffFrequency (SampleType newFrequencyHz)
    {
        filter.template setCutoffFrequency<shouldUpdate> (newFrequencyHz);
    }

    /**
     * Sets the resonance of the filter.
     *
     * Note: The bandwidth of the resonance increases with the value of the
     * parameter. To have a standard 12 dB / octave filter, the value must be set
     * at 1 / sqrt(2).
    */
    template <bool shouldUpdate = true>
    void setQValue (SampleType newResonance)
    {
        filter.template setQValue<shouldUpdate> (newResonance);
    }

    /**
     * Sets the gain of the filter in units of linear gain.
     *
     * Note that for some filter types (Lowpass, Highpass, Bandpass, Allpass)
     * this control will have no effect.
     */
    template <bool shouldUpdate = true>
    void setGain (SampleType newGainLinear)
    {
        filter.template setGain<shouldUpdate> (newGainLinear);
    }

    /**
     * Sets the gain of the filter in units of Decibels.
     *
     * Note that for some filter types (Lowpass, Highpass, Bandpass, Allpass)
     * this control will have no effect.
     */
    template <bool shouldUpdate = true>
    void setGainDecibels (SampleType newGainDecibels)
    {
        filter.template setGainDecibels<shouldUpdate> (newGainDecibels);
    }

    /**
     * Updates the filter coefficients.
     *
     * Don't touch this unless you know what you're doing!
     */
    void update() { filter.update(); }

    /** Turns on the ARP filter's "Limit" mode */
    void setLimitMode (bool shouldLimitModeBeOn) { useLimitMode = shouldLimitModeBeOn; }

    /** Returns the cutoff frequency of the filter. */
    [[nodiscard]] SampleType getCutoffFrequency() const noexcept { return filter.getCutoffFrequency(); }

    /** Returns the resonance of the filter. */
    [[nodiscard]] SampleType getQValue() const noexcept { return filter.getQValue(); }

    /** Returns the gain of the filter. */
    [[nodiscard]] SampleType getGain() const noexcept { return filter.getGain(); }

    /** Initialises the filter. */
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        filter.prepare (spec);
    }

    /** Resets the internal state variables of the filter. */
    void reset() noexcept
    {
        filter.reset();
    }

    /**
     * Ensure that the state variables are rounded to zero if the state
     * variables are denormals. This is only needed if you are doing
     * sample by sample processing.
    */
    void snapToZero() noexcept { filter.snapToZero(); }

    /**
     * Processes a block of samples with a given filter mode.
     *
     * The "notch mix" parameter [-1,1] controls the mix between the notch
     * and other filter types (i.e. -1 = highpass, 0 = notch, 1 = lowpass),
     * and is only active in notch mode.
     */
    template <ARPFilterType type>
    void processBlock (const BufferView<SampleType>& buffer, NumericType notchMix = 0) noexcept
    {
        for (auto [channel, sampleData] : buffer_iters::channels (buffer))
        {
            ScopedValue s1 { filter.ic1eq[(size_t) channel] };
            ScopedValue s2 { filter.ic2eq[(size_t) channel] };
            for (auto& sample : sampleData)
                sample = processSampleInternal<type> (sample, s1.get(), s2.get(), notchMix);
        }

#if JUCE_SNAP_TO_ZERO
        snapToZero();
#endif
    }

    /**
     * Processes a block of samples with a given filter mode.
     *
     * The "notch mix" parameter [-1,1] controls the mix between the notch
     * and other filter types (i.e. -1 = highpass, 0 = notch, 1 = lowpass),
     * and is only active in notch mode.
     */
    template <ARPFilterType type>
    void processBlock (const BufferView<SampleType>& buffer, const float* notchMix) noexcept
    {
        for (auto [channel, sampleData] : buffer_iters::channels (buffer))
        {
            ScopedValue s1 { filter.ic1eq[(size_t) channel] };
            ScopedValue s2 { filter.ic2eq[(size_t) channel] };
            for (auto [i, sample] : enumerate (sampleData))
                sample = processSampleInternal<type> (sample, s1.get(), s2.get(), notchMix[i]);
        }

#if JUCE_SNAP_TO_ZERO
        snapToZero();
#endif
    }

    /**
     * Processes a single sample.
     *
     * The "notch mix" parameter [-1,1] controls the mix between the notch
     * and other filter types (i.e. -1 = highpass, 0 = notch, 1 = lowpass),
     * and is only active in notch mode.
     */
    template <ARPFilterType type>
    inline SampleType processSample (SampleType x, NumericType notchMix = 0, int channel = 0) noexcept
    {
        return processSampleInternal<type> (x, filter.ic1eq[(size_t) channel], filter.ic2eq[(size_t) channel], notchMix);
    }

private:
    template <ARPFilterType type>
    inline SampleType processSampleInternal (SampleType x, SampleType& s1, SampleType& s2, [[maybe_unused]] NumericType notchMix) noexcept
    {
        auto inputMult = useLimitMode ? filter.k0 : SampleType (1);
        const auto [v0, v1, v2] = filter.processCore (inputMult * x, s1, s2);

        juce::ignoreUnused (v0);
        if constexpr (type == FilterType::Lowpass)
            return v2;
        else if constexpr (type == FilterType::Bandpass)
            return v1;
        else if constexpr (type == FilterType::Highpass)
            return v0;
        else if constexpr (type == FilterType::Notch)
        {
            const auto lowMix = notchMix * (NumericType) 0.5 + (NumericType) 0.5;
            const auto highMix = (NumericType) 1 - lowMix;
            const auto makeup = (NumericType) 2 - std::abs (notchMix);

            return makeup * (lowMix * v2 + highMix * v0); // low + high
        }
        else
        {
            jassertfalse; // unknown filter type!
            return {};
        }
    }

    bool useLimitMode = false;
    StateVariableFilter<SampleType, StateVariableFilterType::Lowpass> filter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARPFilter)
};
} // namespace chowdsp