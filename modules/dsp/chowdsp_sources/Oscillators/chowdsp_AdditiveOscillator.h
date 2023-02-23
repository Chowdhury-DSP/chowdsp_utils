#pragma once

namespace chowdsp
{
template <size_t maxNumHarmonics, typename SampleType = float>
class AdditiveOscillator
{
public:
    static_assert (std::is_floating_point_v<SampleType>, "SampleType must be a floating point type!");

    using Vec = xsimd::batch<SampleType>;
    static constexpr auto vecSize = Vec::size;
    static constexpr auto maxNumVecSines = Math::ceiling_divide (maxNumHarmonics, vecSize);

    AdditiveOscillator() = default;

    /** Sets the harmonic amplitudes from an array of amplitude values. */
    void setHarmonicAmplitudes (const SampleType (&amps)[maxNumHarmonics]);

    /** Set's the oscillator's fundamental frequency. */
    void setFrequency (SampleType frequencyHz, bool force = false);

    /** Prepares the filter bank to process a new audio stream */
    void prepare (double sampleRate, int samplesPerBlock);

    /** Resets the oscillator phase */
    void reset (SampleType phase = (SampleType) 0);

    /** Process a single sample of audio. */
    inline SampleType processSample() noexcept
    {
        auto vecFreqMult = iota;

        Vec result {};
        for (auto& amp : amplitudesInternal)
        {
            result += amp * xsimd::sin (vecFreqMult * oscPhase.phase);
            vecFreqMult += (SampleType) vecSize;
        }
        const auto sample = xsimd::reduce_add (result);

        oscPhase.advance (phaseEps);
        return sample;
    }

    /** Process an audio buffer */
    void processBlock (const BufferView<SampleType>& buffer) noexcept;

private:
    std::array<Vec, maxNumVecSines> amplitudesInternal {};
    std::array<Vec, maxNumVecSines> amplitudes {};

    SampleType nyquistFreq = (SampleType) 24000;
    SampleType oscFrequency = (SampleType) 440;

    juce::dsp::Phase<SampleType> oscPhase;
    SampleType phaseEps {};
    SampleType twoPiOverFs {};

    const Vec iota = []
    {
        alignas (xsimd::default_arch::alignment()) SampleType arr[vecSize] {};
        for (size_t i = 0; i < vecSize; ++i)
            arr[i] = SampleType (i + 1);
        return xsimd::load_aligned (arr);
    }();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AdditiveOscillator)
};
} // namespace chowdsp

#include "chowdsp_AdditiveOscillator.cpp"
