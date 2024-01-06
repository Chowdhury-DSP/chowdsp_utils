#pragma once

namespace chowdsp
{
/** Sinusoidal oscillator using the "magic circle" algorithm.
 * See: https://ccrma.stanford.edu/~jos/pasp/Digital_Sinusoid_Generators.html, eq. 2
 */
template <typename T>
class SineWave
{
public:
    using NumericType = SampleTypeHelpers::NumericType<T>;

    SineWave() = default;

    /** Sets the frequency of the oscillator. */
    void setFrequency (T newFrequency) noexcept;

    /** Returns the current frequency of the oscillator. */
    T getFrequency() const noexcept { return freq; }

    /** Prepares the oscillator to process at a given sample rate */
    void prepare (const juce::dsp::ProcessSpec& spec) noexcept;

    /** Resets the internal state of the oscillator */
    void reset() noexcept;

    /** Resets the internal state of the oscillator with an initial phase */
    void reset (T phase) noexcept;

    /** Returns the result of processing a single sample. */
    inline T processSample() noexcept
    {
        auto y = x2;
        x1 += eps * x2;
        x2 -= eps * x1;
        return y;
    }

    /** Returns a single-sample sin/cosine pair. */
    inline auto processSampleQuadrature() noexcept
    {
        return std::make_pair (processSample(), -x1);
    }

    /** Processes a block of samples. */
    void processBlock (const BufferView<T>& buffer) noexcept;

    /** Processes the input and output buffers supplied in the processing context. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept;

private:
    T x1 = static_cast<T> (0.0);
    T x2 = static_cast<T> (0.0);
    T eps = static_cast<T> (0.0);

    T freq = static_cast<T> (0.0);
    T fs = static_cast<T> (44100.0);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SineWave)
};

} // namespace chowdsp

#include "chowdsp_SineWave.cpp"
