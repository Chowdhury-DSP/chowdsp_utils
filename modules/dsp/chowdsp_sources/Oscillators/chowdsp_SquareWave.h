#pragma once

namespace chowdsp
{
/**
 * Square wave following the equation y = sgn(phase), where phase goes from [-1, 1].
 *
 * The wave is generated using "Differentiated Polynomial Waveforms" (DPW),
 * with 2nd-order polynomials.
 *
 * Reference: "Alias-Suppressed Oscillators Based on Differentiated Polynomial Waveforms",
 * Valimaki, Nan, Smith, and Abel.
 * https://mac.kaist.ac.kr/pubs/ValimakiNamSmithAbel-taslp2010.pdf
 */
template <typename T>
class SquareWave
{
public:
    SquareWave() = default;

    /** Sets the frequency of the oscillator. */
    void setFrequency (T newFrequency) noexcept;

    /** Returns the current frequency of the oscillator. */
    T getFrequency() const noexcept { return saw1.getFrequency(); }

    /** Prepares the oscillator to process at a given sample rate */
    void prepare (const juce::dsp::ProcessSpec& spec) noexcept;

    /** Resets the internal state of the oscillator, with a phase in range [-1, 1] */
    void reset (T phase = (T) -1) noexcept;

    /** Returns the result of processing a single sample. */
    inline T processSample() noexcept
    {
        return saw2.processSample() - saw1.processSample();
    }

    /** Processes a block of samples. */
    void processBlock (const BufferView<T>& buffer) noexcept;

#if JUCE_MODULE_AVAILABLE_juce_dsp
    /** Processes the input and output buffers supplied in the processing context. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept;
#endif

private:
    SawtoothWave<T> saw1, saw2;

#if JUCE_MODULE_AVAILABLE_juce_dsp
    juce::HeapBlock<char> dataBlock;
    AudioBlock<T> interMediateData;
#endif
    Buffer<T> intermediateBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SquareWave)
};
} // namespace chowdsp

#include "chowdsp_SquareWave.cpp"
