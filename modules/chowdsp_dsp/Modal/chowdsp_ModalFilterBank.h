#pragma once

namespace chowdsp
{
/** A parallel bank of modal filters, with SIMD vectorization */
template <int maxNumModes, typename SampleType = float>
class ModalFilterBank
{
    static_assert (std::is_floating_point_v<SampleType>, "SampleType must be a floating point type!");

public:
    using Vec = juce::dsp::SIMDRegister<SampleType>;
    static constexpr auto vecSize = (int) Vec::size();
    static constexpr auto maxNumVecModes = ceiling_divide (maxNumModes, vecSize);

    ModalFilterBank() = default;

    /**
     * Sets the mode amplitudes from an array of split real/imaginary amplitude values.
     *
     * "Normalize" will set the level of the first mode, or you may pass a negative
     * normalization constant to disable normalization.
     */
    void setModeAmplitudes (const SampleType (&ampsReal)[maxNumModes], const SampleType (&ampsImag)[maxNumModes], SampleType normalize = 1.0f);

    /**
     * Sets the mode amplitudes from an array of complex amplitude values.
     *
     * "Normalize" will set the level of the first mode, or you may pass a negative
     * normalization constant to disable normalization.
     */
    void setModeAmplitudes (const std::complex<SampleType> (&amps)[maxNumModes], SampleType normalize = 1.0f);

    /** Sets the mode frequencies */
    void setModeFrequencies (const SampleType (&baseFrequencies)[maxNumModes], SampleType frequencyMultiplier = (SampleType) 1);

    /** Sets the mode decay rates from a set of "tau" values */
    void setModeDecays (const SampleType (&baseTaus)[maxNumModes], SampleType originalSampleRate, SampleType decayFactor = (SampleType) 1);

    /** Sets the mode decay rates in units of T60 */
    void setModeDecays (const SampleType (&t60s)[maxNumModes]);

    /** Selects the number of modes to process */
    void setNumModesToProcess (int numModesToProcess);

    /** Prepares the filter bank to process a new audio stream */
    void prepare (double sampleRate, int samplesPerBlock);

    /** Resets the filter bank state */
    void reset();

    /** Processes a single (vectorized) mode. Note that modeIndex is relative the number of vectorized modes */
    inline auto processSample (size_t modeIndex, SampleType x) noexcept
    {
        return modes[modeIndex].processSample (x);
    }

    /** Process an audio buffer */
    void process (const juce::AudioBuffer<SampleType>& buffer) noexcept;

    /** Process an audio block */
    void process (const juce::dsp::AudioBlock<const SampleType>& block) noexcept;

    /** Process with some user-defined modulator */
    template <typename Modulator>
    void processWithModulation (const juce::dsp::AudioBlock<const SampleType>& block, Modulator&& modulator) noexcept;

    /** Returns a mono buffer of rendered audio */
    const auto& getRenderBuffer() const noexcept { return renderBuffer; }

private:
    template <typename PerModeFunc, typename PerVecModeFunc>
    void doForModes (PerModeFunc&& perModeFunc, PerVecModeFunc&& perVecModeFunc);
    static Vec tau2t60 (Vec tau, SampleType originalSampleRate);

    void updateAmplitudeNormalizationFactor (SampleType normalize);
    void setModeAmplitudesInternal();

    std::array<chowdsp::ModalFilter<Vec>, maxNumVecModes> modes;

    std::array<std::complex<SampleType>, maxNumModes> amplitudeData;
    SampleType amplitudeNormalizationFactor = (SampleType) 1;

    juce::AudioBuffer<SampleType> renderBuffer;
    SampleType maxFreq = (SampleType) 0;
    int numModesToProcess = maxNumModes;
    int numVecModesToProcess = maxNumVecModes;

    const static SampleType log1000;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModalFilterBank)
};
} // namespace chowdsp

#include "chowdsp_ModalFilterBank.cpp"
