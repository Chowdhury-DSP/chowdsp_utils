#pragma once

namespace juce::dsp
{
/** Copy of juce::dsp::ProcessSpec */
struct ProcessSpec
{
    /** The sample rate that will be used for the data that is sent to the processor. */
    double sampleRate;

    /** The maximum number of samples that will be in the blocks sent to process() method. */
    size_t maximumBlockSize;

    /** The number of channels that the process() method will be expected to handle. */
    size_t numChannels;
};
} // namespace juce::dsp
