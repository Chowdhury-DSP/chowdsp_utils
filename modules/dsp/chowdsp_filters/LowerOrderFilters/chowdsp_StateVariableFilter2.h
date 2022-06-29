#pragma once

namespace chowdsp
{
/** Filter type options for State Variable Filters */
enum class StateVariableFilter2Type
{
    Lowpass,
    Bandpass,
    Highpass,
    Notch,
    Bell,
    LowShelf,
    HighShelf,
};

template <typename SampleType, StateVariableFilter2Type type>
class StateVariableFilter2
{
public:
    static constexpr int Order = 2;
    static constexpr auto Type = type;

    using FilterType = StateVariableFilter2Type;
    using NumericType = SampleTypeHelpers::NumericType<SampleType>;

    /** Constructor. */
    StateVariableFilter2();

    /**
     * Sets the cutoff frequency of the filter.
     *
     * @param newFrequencyHz the new cutoff frequency in Hz.
    */
    void setCutoffFrequency (SampleType newFrequencyHz);

    /**
     * Sets the resonance of the filter.
     *
     * Note: The bandwidth of the resonance increases with the value of the
     * parameter. To have a standard 12 dB / octave filter, the value must be set
     * at 1 / sqrt(2).
    */
    void setQValue (SampleType newResonance);

    /** Returns the cutoff frequency of the filter. */
    [[nodiscard]] SampleType getCutoffFrequency() const noexcept { return cutoffFrequency; }

    /** Returns the resonance of the filter. */
    [[nodiscard]] SampleType getResonance() const noexcept { return resonance; }

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

    /** Processes one sample at a time on a given channel. */
    inline SampleType processSample (int channel, SampleType inputValue) noexcept
    {
        return processSampleInternal (inputValue, ic1eq[(size_t) channel], ic2eq[(size_t) channel]);
    }

private:
    inline SampleType processSampleInternal (SampleType x, SampleType& s1, SampleType& s2) noexcept
    {
        const auto v3 = x - s2;
        const auto v0 = a1 * v3 - ak * s1;
        const auto v1 = a2 * v3 + a1 * s1;
        const auto v2 = a3 * v3 + a2 * s1 + s2;

        // update state
        s1 = (NumericType) 2 * v1 - s1;
        s2 = (NumericType) 2 * v2 - s2;

        if constexpr (type == FilterType::Lowpass)
            return v2;
        else if constexpr (type == FilterType::Bandpass)
            return v1;
        else if constexpr (type == FilterType::Highpass)
            return v0;
        else if constexpr (type == FilterType::Notch)
            return v0 + v2;
        else
        {
            jassertfalse; // unknown filter type!
            return {};
        }
    }

    void update();

    SampleType cutoffFrequency, resonance, gain; // parameters
    SampleType g0, k0, A; // parameter intermediate values
    SampleType a1, a2, a3, ak; // coefficients
    //    SampleType g, k, Asq, k0A, gk, gt0, gk0; // coefficients (mostly)
    std::vector<SampleType> ic1eq { 2 }, ic2eq { 2 }; // state variables

    double sampleRate = 44100.0;
};
} // namespace chowdsp

#include "chowdsp_StateVariableFilter2.cpp"
