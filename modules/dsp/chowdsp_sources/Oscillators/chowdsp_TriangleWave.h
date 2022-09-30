#pragma once

namespace chowdsp
{
/**
 * Triangle wave following the equation y = 1 - 2 * |phase|, where phase goes from [-1, 1].
 *
 * The wave is generated using "Differentiated Polynomial Waveforms" (DPW),
 * with 2nd-order polynomials.
 *
 * Reference: "Alias-Suppressed Oscillators Based on Differentiated Polynomial Waveforms",
 * Valimaki, Nan, Smith, and Abel.
 * https://mac.kaist.ac.kr/pubs/ValimakiNamSmithAbel-taslp2010.pdf
 */
template <typename T>
class TriangleWave
{
public:
    TriangleWave() = default;

    /** Sets the frequency of the oscillator. */
    void setFrequency (T newFrequency) noexcept;

    /** Returns the current frequency of the oscillator. */
    T getFrequency() const noexcept { return freq; }

    /** Prepares the oscillator to process at a given sample rate */
    void prepare (const juce::dsp::ProcessSpec& spec) noexcept;

    /** Resets the internal state of the oscillator, with a phase in range [-1, 1] */
    void reset (T phase = (T) -1) noexcept;

    /** Returns the result of processing a single sample. */
    inline T processSample() noexcept
    {
        return processSampleInternal (z, phi);
    }

    /** Processes a block of samples. */
    void processBlock (const BufferView<T>& buffer) noexcept;

private:
    inline T processSampleInternal (T& state, T& phase)
    {
        CHOWDSP_USING_XSIMD_STD (abs);

        // anti-alias with DPW
        auto adWave = phase * abs (phase) - phase; // 2nd-order polynomial of anti-derivative
        auto y = adWave - state; // differentiate
        state = adWave; // update state

        updatePhase (phase, deltaPhase);

        return waveformPreservingScale * y;
    }

    static inline void updatePhase (T& phi, const T& deltaPhase) noexcept
    {
        using namespace SIMDUtils;
        phi += deltaPhase;
        phi = SIMDUtils::select (phi >= (T) 1, phi - (T) 2, phi);
    }

    T z {};
    T phi {};

    T deltaPhase {};
    T waveformPreservingScale {};

    T freq = static_cast<T> (0.0);
    T fs = static_cast<T> (44100.0);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TriangleWave)
};
} // namespace chowdsp

#include "chowdsp_TriangleWave.cpp"
