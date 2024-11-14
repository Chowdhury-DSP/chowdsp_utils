#pragma once

#if ! CHOWDSP_NO_XSIMD

namespace chowdsp
{
/** Sine approximation type to use for the AdditiveOscillator */
enum class AdditiveOscSineApprox
{
    AbsApprox, /**< Approximate sine using y = phi * (pi - abs(phi)). ~3.6x faster than FullPrecision, aliasing artifacts up to -33 dB */
    BhaskaraApprox, /**< Approximate sine using Bhaskara's approximation. ~2.75x faster than FullPrecision, aliasing artifacts up to -60 dB */
    FullPrecision, /**< Sine function with equivalent precision to std::sin */
};

/** An additive oscillator with some number of harmonics. */
template <size_t maxNumHarmonics, AdditiveOscSineApprox sineApprox = AdditiveOscSineApprox::BhaskaraApprox, typename SampleType = float>
class AdditiveOscillator
{
public:
    static_assert (std::is_floating_point_v<SampleType>, "SampleType must be a floating point type!");

    static constexpr auto numHarmonics = maxNumHarmonics;
    using Vec = xsimd::batch<SampleType>;
    static constexpr auto vecSize = Vec::size;
    static constexpr auto maxNumVecSines = Math::ceiling_divide (maxNumHarmonics, vecSize);

    AdditiveOscillator() = default;

    /** Sets the harmonic amplitudes from an array of amplitude values. */
    void setHarmonicAmplitudes (const SampleType (&amps)[maxNumHarmonics]);

    /** Sets the harmonic amplitudes from an array of amplitude values. */
    void setHarmonicAmplitudes (nonstd::span<const SampleType> amps);

    /** Set's the oscillator's fundamental frequency. Harmonics above Nyquist will be automatically filtered out. */
    void setFrequency (SampleType frequencyHz, bool force = false);

    /** Prepares the filter bank to process a new audio stream */
    void prepare (double sampleRate);

    /** Resets the oscillator phase */
    void reset (SampleType phase = (SampleType) 0);

    /** Process a single sample of audio. */
    inline SampleType processSample() noexcept
    {
        const auto sample = generateSample (iota, oscPhase, amplitudesInternal);
        incrementPhase (oscPhase, phaseEps);
        return sample;
    }

    /** Process an audio buffer */
    void processBlock (const BufferView<SampleType>& buffer) noexcept;

private:
    static inline void incrementPhase (SampleType& phase, SampleType eps) noexcept
    {
        phase += eps;
        phase = phase > juce::MathConstants<SampleType>::pi ? (phase - juce::MathConstants<SampleType>::twoPi) : phase;
    }

    static inline auto generateSample (Vec iota, SampleType phase, const std::array<Vec, maxNumVecSines>& amps) noexcept
    {
        Vec result {};
        for (auto& amp : amps)
        {
            result += amp * sineFunction (iota * phase);
            iota += (SampleType) vecSize;
        }
        return xsimd::reduce_add (result);
    }

    template <AdditiveOscSineApprox A = sineApprox>
    static inline std::enable_if_t<A == AdditiveOscSineApprox::AbsApprox, Vec> sineFunction (Vec phi) noexcept
    {
        static constexpr auto pi = juce::MathConstants<SampleType>::pi;
        static constexpr auto twoPi = juce::MathConstants<SampleType>::twoPi;
        static constexpr auto fourOverPiSq = (SampleType) 4 / (pi * pi);

        const auto phiMod = xsimd::fmod (phi + maxNumHarmonics * twoPi, (Vec) twoPi) - pi;
        const auto absPhi = xsimd::abs (phiMod);

        return -fourOverPiSq * phiMod * (pi - absPhi);
    }

    template <AdditiveOscSineApprox A = sineApprox>
    static inline std::enable_if_t<A == AdditiveOscSineApprox::BhaskaraApprox, Vec> sineFunction (Vec phi) noexcept
    {
        static constexpr auto pi = juce::MathConstants<SampleType>::pi;
        static constexpr auto twoPi = juce::MathConstants<SampleType>::twoPi;
        static constexpr auto fivePiSq = (SampleType) 5 * pi * pi;

        const auto phiMod = xsimd::fmod (phi + maxNumHarmonics * twoPi, (Vec) twoPi) - pi;
        const auto signPhi = Math::sign (phiMod);
        const auto absPhi = xsimd::abs (phiMod);

        const auto absPhiTimesPiMinusPhi = absPhi * (pi - absPhi);
        const auto numerator = (SampleType) 16 * absPhiTimesPiMinusPhi;
        const auto denominator = fivePiSq - (SampleType) 4 * absPhiTimesPiMinusPhi;
        return -signPhi * numerator / denominator;
    }

    template <AdditiveOscSineApprox A = sineApprox>
    static inline std::enable_if_t<A == AdditiveOscSineApprox::FullPrecision, Vec> sineFunction (Vec phi) noexcept
    {
        return xsimd::sin (phi);
    }

    std::array<Vec, maxNumVecSines> amplitudesInternal {};
    std::array<Vec, maxNumVecSines> amplitudes {};

    SampleType nyquistFreq = (SampleType) 24000;
    SampleType oscFrequency = (SampleType) 440;

    SampleType oscPhase {};
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

#endif // ! CHOWDSP_NO_XSIMD
