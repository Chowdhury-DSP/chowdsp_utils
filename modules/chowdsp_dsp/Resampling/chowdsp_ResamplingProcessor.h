#pragma once

namespace chowdsp
{
/** A resampler using an internal resampling type
 *  as defined in the ResamplingTypes namespace.
 * 
 *  Note that the number of samples returned from the resampler
 *  may not be enough to fill the required output buffer, so it
 *  might be necessary to buffer some extra samples.
 */
template <typename ResamplerType>
class ResamplingProcessor
{
public:
    static_assert (std::is_base_of<ResamplingTypes::BaseResampler, ResamplerType>::value, "ResamplerType must be derived from BaseResampler");

    /** Default constructor */
    ResamplingProcessor() = default;

    /** Prepares the resampler to process a new stream of data */
    void prepare (const juce::dsp::ProcessSpec& spec, double startRatio = 1.0)
    {
        resamplers = std::vector<ResamplerType> (spec.numChannels);
        for (auto& r : resamplers)
            r.prepare (spec.sampleRate, startRatio);

        outputBuffer.setSize ((int) spec.numChannels,
                              (int) spec.maximumBlockSize * 20);
    }

    /** Resets the state of the resampler */
    void reset()
    {
        for (auto& r : resamplers)
            r.reset();
    }

    /** Sets the ratio of output sample rate over input sample rate
     * 
     *  @param ratio    The resampling ratio. Must be in [0.1, 10.0]
     */
    void setResampleRatio (float ratio)
    {
        auto ratioClamped = juce::jlimit (0.01f, 100.0f, ratio);
        for (auto& r : resamplers)
            r.setResampleRatio (ratioClamped);
    }

    /** Returns the ratio of output sample rate over input sample rate */
    float getResampleRatio() const noexcept
    {
        return resamplers[0].getResampleRatio();
    }

    /** Processes an input block of samples
     * 
     *  @return the output block of generated samples at the new sample rate
     */
    juce::dsp::AudioBlock<float> process (const juce::dsp::AudioBlock<float>& block) noexcept
    {
        const auto numChannels = block.getNumChannels();
        const auto numSamples = block.getNumSamples();

        size_t outNumSamples = 0;
        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            outNumSamples = resamplers[ch].process (block.getChannelPointer (ch),
                                                    outputBuffer.getWritePointer ((int) ch),
                                                    numSamples);
        }

        return juce::dsp::AudioBlock<float> (outputBuffer).getSubBlock (0, outNumSamples);
    }

private:
    std::vector<ResamplerType> resamplers;
    juce::AudioBuffer<float> outputBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResamplingProcessor)
};

} // namespace chowdsp
