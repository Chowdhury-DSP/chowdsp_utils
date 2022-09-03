#pragma once

namespace chowdsp::experimental
{
/**
 * Oscillator based on continuous-ordered polygons. Still a little bit of a WIP.
 *
 * Reference: https://quod.lib.umich.edu/cgi/p/pod/dod-idx/continuous-order-polygonalwaveform-synthesis.pdf?c=icmc;idno=bbp2372.2016.104;format=pdf
 */
template <typename FloatType>
class PolygonalOscillator
{
    using NumericType = SampleTypeHelpers::NumericType<FloatType>;

public:
    PolygonalOscillator() = default;

    /** Sets the frequency of the oscillator. */
    void setFrequency (FloatType newFrequency) noexcept;

    /** Returns the current frequency of the oscillator. */
    FloatType getFrequency() const noexcept { return freq; }

    /** Sets the polygonal order of the oscillator. */
    void setOrder (FloatType newOrder) noexcept;

    /** Returns the current order of the oscillator. */
    FloatType getOrder() const noexcept { return order; }

    /** Sets the polygonal "teeth" of the oscillator. */
    void setTeeth (FloatType newTeeth) noexcept;

    /** Returns the current frequency of the oscillator. */
    FloatType getTeeth() const noexcept { return teeth; }

    /** Prepares the oscillator to process at a given sample rate */
    void prepare (const juce::dsp::ProcessSpec& spec) noexcept;

    /** Resets the internal state of the oscillator */
    void reset() noexcept;

    /** Resets the internal state of the oscillator with an initial phase */
    void reset (FloatType phase) noexcept;

    /** Returns the result of processing a single sample. */
    inline FloatType processSample() noexcept
    {
        CHOWDSP_USING_XSIMD_STD (cos);
        CHOWDSP_USING_XSIMD_STD (sin);
        CHOWDSP_USING_XSIMD_STD (fmod);
        static constexpr auto twoPi = juce::MathConstants<NumericType>::twoPi;

        // @TODO: implement some form of aliasing suppression, maybe polyBLAMP?
        // @TODO: optimize! (magic circle?)
        auto y = cosPiOverOrder * sin (phase);
        y /= cos (rOrder * fmod (phase * order, static_cast<FloatType> (twoPi)) - juce::MathConstants<NumericType>::pi * rOrder + teeth);

        phase += deltaPhase;
        phase = SIMDUtils::select (phase >= twoPi, phase - twoPi, phase);

        return y;
    }

    /** Processes a block of samples. */
    void processBlock (const BufferView<FloatType>& buffer) noexcept;

private:
    FloatType freq = {};
    NumericType fs = (NumericType) 48000;

    FloatType order = (FloatType) 2;
    FloatType rOrder = (FloatType) 1 / order;
    FloatType cosPiOverOrder = {};
    FloatType teeth = {};

    FloatType phase = {};
    FloatType deltaPhase = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolygonalOscillator)
};
} // namespace chowdsp::experimental

#include "chowdsp_PolygonalOscillator.cpp"
