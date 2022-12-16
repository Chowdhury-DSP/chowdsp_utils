#pragma once

namespace chowdsp
{
/** A simple level detector, that I like better than the JUCE one */
template <typename SampleType>
class LevelDetector
{
public:
    LevelDetector() = default;
    LevelDetector (LevelDetector&&) noexcept = default;
    LevelDetector& operator= (LevelDetector&&) noexcept = default;
    virtual ~LevelDetector() = default;

    /** Sets the current time constants, in milliseconds. */
    void setParameters (float attackTimeMs, float releaseTimeMs);

    /** Initialises the processor. */
    void prepare (const juce::dsp::ProcessSpec& spec);

    /** Resets the internal state variables of the processor. */
    void reset();

    /**
     * Processes a buffer of audio data in-place.
     * If the buffer contains more than one channel,
     * the output level will be stored in the first channel.
     */
    void processBlock (const BufferView<SampleType>& buffer) noexcept;

    /**
     * Processes a buffer of audio data in-place.
     * If the output buffer contains more than one channel,
     * the output level will be stored in the first channel.
     */
    void processBlock (const BufferView<const SampleType>& bufferIn, const BufferView<SampleType>& bufferOut) noexcept;

    /** Processes the input and output samples supplied in the processing context. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept;

    /** Processes a single sample. Note that this function expects the input to be non-negative */
    virtual inline SampleType processSample (SampleType x) noexcept
    {
        return processSampleInternal (x, increasing, yOld);
    }

protected:
    float expFactor = 1.0f;
    SampleType yOld;
    bool increasing = true;

    SampleType tauAtt = (SampleType) 1;
    SampleType tauRel = (SampleType) 1;

private:
    inline SampleType processSampleInternal (SampleType x, bool& _increasing, SampleType& _yOld) noexcept
    {
        auto tau = _increasing ? tauAtt : tauRel;
        x = _yOld + tau * (x - _yOld);

        // update for next sample
        _increasing = x > _yOld;
        _yOld = x;

        return x;
    }

    Buffer<SampleType> absBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelDetector)
};

} // namespace chowdsp

#include "chowdsp_LevelDetector.cpp"
