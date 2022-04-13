#pragma once

namespace chowdsp
{
/** Divides two numbers and rounds up if there is a remainder. */
template <typename T>
constexpr T ceiling_divide(T num, T den)
{
    return (num + den - 1) / den;
}

template <int maxNumModes, typename SampleType = float>
class ModalFilterBank
{
    static_assert (std::is_floating_point_v<SampleType>, "SampleType must be a floating point type!");

public:
    using Vec = juce::dsp::SIMDRegister<SampleType>;
    static constexpr auto vecSize = (int) Vec::size();
    static constexpr auto maxNumVecModes = ceiling_divide (maxNumModes, vecSize);

    ModalFilterBank() = default;

    void setModeAmplitudes (const SampleType (&ampsReal)[maxNumModes], const SampleType (&ampsImag)[maxNumModes], SampleType normalize = 1.0f);
    void setModeAmplitudes (const std::complex<SampleType> (&amps)[maxNumModes], SampleType normalize = 1.0f);
    void setModeFrequencies (const SampleType (&baseFrequencies)[maxNumModes], SampleType frequencyMultiplier = (SampleType) 1);
    void setModeDecays (const SampleType (&baseTaus)[maxNumModes], SampleType originalSampleRate, SampleType decayFactor = (SampleType) 1);
    void setModeDecays (const SampleType (&t60s)[maxNumModes]);

    void setNumModesToProcess (int numModesToProcess);

    void prepare (double sampleRate, int samplesPerBlock);
    void reset();

    inline auto processSample (size_t modeIndex, SampleType x) noexcept
    {
        return modes[modeIndex].processSample (x);
    }

    void process (const juce::AudioBuffer<SampleType>& buffer) noexcept;
    void process (const juce::dsp::AudioBlock<const SampleType>& block) noexcept;

    template <typename Modulator>
    void processWithModulation (const juce::dsp::AudioBlock<const SampleType>& block, Modulator&& modulator) noexcept;

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
}

#include "chowdsp_ModalFilterbank.cpp"
