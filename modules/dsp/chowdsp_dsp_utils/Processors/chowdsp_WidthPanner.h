#pragma once

#include "chowdsp_Panner.h"

namespace chowdsp
{
/**
 * A panner-style processor that affects the "width" of the
 * processed signal.
 */
template <typename SampleType>
class WidthPanner
{
public:
    WidthPanner()
    {
        setRule (rule);
        setPan (1.0f);
    }

    /** Sets the panning rule. */
    void setRule (PanningRule newRule)
    {
        rule = newRule;
        leftPanner.setRule (rule);
        rightPanner.setRule (rule);
    }

    /** Sets the current panning value, between 1 (stereo), 0 (mono) and -1 (inverted stereo). */
    void setPan (SampleType newPan)
    {
        leftPanner.setPan (-newPan);
        rightPanner.setPan (newPan);
    }

    /** Initialises the processor. */
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        leftPanner.prepare (spec);
        rightPanner.prepare (spec);

        leftPanBuffer.setMaxSize (2, static_cast<int> (spec.maximumBlockSize));
    }

    /** Resets the internal state variables of the processor. */
    void reset()
    {
        leftPanner.reset();
        rightPanner.reset();
    }

    /** Processes a stereo buffer. */
    void processBlock (const BufferView<SampleType>& buffer) noexcept
    {
        jassert (buffer.getNumChannels() == 2);

        const auto numSamples = buffer.getNumSamples();
        leftPanBuffer.setCurrentSize (2, numSamples);

        // copy left signal into both channels of leftPanBuffer
        for (int ch = 0; ch < 2; ++ch)
            BufferMath::copyBufferChannels (buffer, leftPanBuffer, 0, ch);

        // copy right signal into both channels of buffer
        BufferMath::copyBufferChannels (buffer, buffer, 1, 0);

        leftPanner.processBlock (leftPanBuffer);
        rightPanner.processBlock (buffer);

        BufferMath::addBufferData (leftPanBuffer, buffer);

        BufferMath::applyGain (buffer, static_cast<SampleType> (1) / Panner<SampleType>::getBoostForRule (rule));
    }

private:
    PanningRule rule = PanningRule::linear;

    Panner<SampleType> leftPanner;
    Panner<SampleType> rightPanner;

    Buffer<SampleType> leftPanBuffer {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WidthPanner)
};
} // namespace chowdsp
