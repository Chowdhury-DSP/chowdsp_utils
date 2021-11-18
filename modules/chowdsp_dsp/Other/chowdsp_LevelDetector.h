#pragma once

namespace chowdsp
{
/** A simple level detector, that I like better than the JUCE one */
template <typename SampleType>
class LevelDetector
{
public:
    LevelDetector() = default;

    /** Sets the current time constants, in milliseconds. */
    void setParameters (float attackTimeMs, float releaseTimeMs);

    /** Initialises the processor. */
    void prepare (const juce::dsp::ProcessSpec& spec);

    /** Resets the internal state variables of the processor. */
    void reset();

    /** Processes the input and output samples supplied in the processing context. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept;

    /** Processes a single sample. Note that this function expects the input to be non-negative */
    virtual inline SampleType processSample (SampleType x) noexcept
    {
        auto tau = increasing ? tauAtt : tauRel;
        x = yOld + tau * (x - yOld);

        // update for next sample
        increasing = x > yOld;
        yOld = x;

        return x;
    }

protected:
    float expFactor;
    SampleType yOld;
    bool increasing;

    SampleType tauAtt = (SampleType) 1;
    SampleType tauRel = (SampleType) 1;

private:
    juce::AudioBuffer<SampleType> absBuffer;
    juce::dsp::AudioBlock<SampleType> absBlock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelDetector)
};

} // namespace chowdsp

#include "chowdsp_LevelDetector.cpp"
