#pragma once

namespace chowdsp
{
/** Base class for an audio source that gets repitched. */
template <typename ResamplingType>
class RepitchedSource
{
public:
    /** Constructs a repitched source with a maximum pitch change factor */
    explicit RepitchedSource (float maxPitchChangeFactor);

    /** Sets a new repitch factor for the source */
    void setRepitchFactor (float newRepitchFactor);

    /** Returns the current repitch factor */
    [[nodiscard]] float getRepitchFactor() const noexcept { return 1.0f / resampler.getResampleRatio(); }

    /** Prepares the repitch source to process a new audio stream */
    void prepare (const juce::dsp::ProcessSpec& spec);

    /** Resets the state of the source */
    void reset();

    /** Generates a new block of samples from the source */
    BufferView<float> process (int numSamples) noexcept;

protected:
    /** Override this method to prepare the source */
    virtual void prepareRepitched (const juce::dsp::ProcessSpec&) {}

    /** Override this method to implement custom reset behaviour */
    virtual void resetRepitched() {}

    /** Override this method to run the process which will be repitched */
    virtual void processRepitched (const BufferView<float>&) = 0;

private:
    ResamplingProcessor<ResamplingType> resampler;
    Buffer<float> resampledBuffer;

    const float maxRepitchFactor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RepitchedSource)
};
} // namespace chowdsp

#include "chowdsp_RepitchedSource.cpp"
