#pragma once

namespace chowdsp
{
/**
 * Sawtooth wave following the equation y = phase, where phase goes from [-1, 1].
 *
 * The wave is generated using "Differentiated Polynomial Waveforms" (DPW),
 * with 2nd-order polynomials.
 *
 * Reference: "Alias-Suppressed Oscillators Based on Differentiated Polynomial Waveforms",
 * Valimaki, Nan, Smith, and Abel.
 * https://mac.kaist.ac.kr/pubs/ValimakiNamSmithAbel-taslp2010.pdf
 */
template <typename T>
class SawtoothWave
{
public:
    SawtoothWave() = default;

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
        // anti-alias with DPW
        auto adWave = phi * phi; // 2nd-order polynomial of anti-derivative
        auto y = adWave - z; // differentiate
        z = adWave; // update state

        updatePhase();

        return waveformPreservingScale * y;
    }

    /** Processes a block of samples. */
    void processBlock (const BufferView<T>& buffer) noexcept;

    /** Processes the input and output buffers supplied in the processing context. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept;

private:
    inline void updatePhase() noexcept
    {
        phi += deltaPhase;
        phi = SIMDUtils::select (phi >= (T) 1, phi - (T) 2, phi);
    }

    T z {};
    T phi {};

    T deltaPhase {};
    T waveformPreservingScale {};

    T freq = static_cast<T> (0.0);
    T fs = static_cast<T> (44100.0);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SawtoothWave)
};
} // namespace chowdsp

#include "chowdsp_SawtoothWave.cpp"
