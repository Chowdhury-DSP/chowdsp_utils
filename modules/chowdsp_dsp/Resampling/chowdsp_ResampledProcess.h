#pragma once

namespace chowdsp
{
/** Processor for up/downsampling a signal by a non-integer factor */
template <typename ResamplerType>
class ResampledProcess
{
public:
    ResampledProcess() = default;

    /** Prepares the resampler to process a new stream of data */
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        inputResampler.prepare (spec);
        outputResampler.prepare (spec);

        leftoverBuffer.resize (spec.numChannels, 0.0f);
        leftoverAvailable = false;

        baseFs = (float) spec.sampleRate;
    }

    /** Resets the state of the resampler */
    void reset()
    {
        inputResampler.reset();
        outputResampler.reset();

        std::fill (leftoverBuffer.begin(), leftoverBuffer.end(), 0.0f);
        leftoverAvailable = false;
    }

    /** Sets the ratio of target sample rate over input sample rate
     * 
     *  @param ratio    The resampling ratio. Must be in [0.1, 10.0]
     */
    void setResampleRatio (float ratio)
    {
        inputResampler.setResampleRatio (ratio);
        outputResampler.setResampleRatio (1.0f / ratio);
    }

    /** Sets the target sample rate directly */
    void setTargetSampleRate (float targetSampleRate)
    {
        setResampleRatio (targetSampleRate / baseFs);
    }

    /** Returns the ratio of output sample rate over input sample rate */
    float getResampleRatio() const noexcept
    {
        return inputResampler.getResampleRatio();
    }

    /** Returns the target sample rate */
    float getTargetSampleRate() const noexcept
    {
        return getResampleRatio() * baseFs;
    }

    /** Processes an input block of samples
     * 
     *  @return the output block of generated samples at the target sample rate
     */
    juce::dsp::AudioBlock<float> processIn (const juce::dsp::AudioBlock<float>& block) noexcept
    {
        return inputResampler.process (block);
    }

    void processOut (const juce::dsp::AudioBlock<float>& inBlock, juce::dsp::AudioBlock<float>& outputBlock)
    {
        auto outBlockTemp = outputResampler.process (inBlock);

        const auto availableSamples = (int) outBlockTemp.getNumSamples();
        auto expectedSamples = (int) outputBlock.getNumSamples();
        int destStart = 0;

        if (std::abs (availableSamples - expectedSamples) > 1)
        {
            // we might have a few buffers that are severely under-run
            // when starting to process with a new sample rate.
            // For now, it's okay to clear those out, but if this branch
            // is being hit regularly during processing, then something
            // must be wrong!
            outputBlock.clear();
            return;
        }

        if (leftoverAvailable) // pop leftover samples
        {
            for (int ch = 0; ch < (int) outputBlock.getNumChannels(); ++ch)
            {
                auto* destData = outputBlock.getChannelPointer ((size_t) ch);
                destData[0] = leftoverBuffer[ch];
            }

            destStart = 1;
            expectedSamples -= 1;
            leftoverAvailable = false;
        }

        // do we have exactly the right number of samples to fill the output buffer?
        if (availableSamples == expectedSamples)
        {
            for (int ch = 0; ch < (int) outputBlock.getNumChannels(); ++ch)
            {
                const auto* srcData = outBlockTemp.getChannelPointer ((size_t) ch);
                auto* destData = outputBlock.getChannelPointer ((size_t) ch);
                juce::FloatVectorOperations::copy (destData + destStart, srcData, availableSamples);
            }
            return;
        }

        // we have some extra samples after the buffer is filled!
        if (availableSamples > expectedSamples)
        {
            for (int ch = 0; ch < (int) outputBlock.getNumChannels(); ++ch)
            {
                const auto* srcData = outBlockTemp.getChannelPointer ((size_t) ch);
                auto* destData = outputBlock.getChannelPointer ((size_t) ch);
                juce::FloatVectorOperations::copy (destData + destStart, srcData, expectedSamples);

                leftoverBuffer[ch] = srcData[availableSamples - 1];
            }

            leftoverAvailable = true;
            return;
        }
    }

private:
    ResamplingProcessor<ResamplerType> inputResampler;
    ResamplingProcessor<ResamplerType> outputResampler;

    float baseFs = 48000.0f;

    std::vector<float> leftoverBuffer;
    bool leftoverAvailable = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResampledProcess)
};
} // namespace chowdsp
