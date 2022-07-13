#pragma once

namespace juce::dsp
{
/** Copy of juce::dsp::ProcessSpec */
struct ProcessSpec
{
    /** The sample rate that will be used for the data that is sent to the processor. */
    double sampleRate;

    /** The maximum number of samples that will be in the blocks sent to process() method. */
    uint32_t maximumBlockSize;

    /** The number of channels that the process() method will be expected to handle. */
    uint32_t numChannels;

    // Note: I'd prefer to use `int` for the integer types, but we need `uint32_t` for compatibility with JUCE.
};
} // namespace juce::dsp
