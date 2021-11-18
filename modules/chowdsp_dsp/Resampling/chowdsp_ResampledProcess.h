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
        delayBuffer.prepare (spec);
        delayBuffer.setDelay (0.0f);

        delayCount = 0;
        firstPass = true;

        baseFs = (float) spec.sampleRate;
    }

    /** Resets the state of the resampler */
    void reset()
    {
        inputResampler.reset();
        outputResampler.reset();
        delayBuffer.reset();

        delayCount = 0;
        firstPass = true;
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

        // get old samples from the delay buffer first
        if (delayCount > 0)
        {
            for (int ch = 0; ch < (int) outputBlock.getNumChannels(); ++ch)
            {
                auto* destData = outputBlock.getChannelPointer ((size_t) ch);

                for (int n = 0; n < delayCount; ++n)
                    destData[n] = delayBuffer.popSample (ch);
            }

            destStart += delayCount;
            expectedSamples -= delayCount;
            delayCount = 0;
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
            const auto diff = availableSamples - expectedSamples;
            for (int ch = 0; ch < (int) outputBlock.getNumChannels(); ++ch)
            {
                const auto* srcData = outBlockTemp.getChannelPointer ((size_t) ch);
                auto* destData = outputBlock.getChannelPointer ((size_t) ch);
                juce::FloatVectorOperations::copy (destData + destStart, srcData, expectedSamples);

                for (int n = 0; n < diff; ++n)
                    delayBuffer.pushSample (ch, srcData[destStart + expectedSamples + n]);
            }

            delayCount += diff;
            return;
        }

        // we don't have enough samples available to fill the buffer!
        {
            // this should only happen the first time!
            jassert (firstPass);

            const auto diff = expectedSamples - availableSamples;
            for (int ch = 0; ch < (int) outputBlock.getNumChannels(); ++ch)
            {
                const auto* srcData = outBlockTemp.getChannelPointer ((size_t) ch);
                auto* destData = outputBlock.getChannelPointer ((size_t) ch);

                // front-pad with zeros
                juce::FloatVectorOperations::fill (destData + destStart, 0.0f, diff + 1);
                juce::FloatVectorOperations::copy (destData + destStart + diff, srcData, availableSamples - 1);

                delayBuffer.pushSample (ch, srcData[availableSamples - 1]);
            }

            firstPass = false;
            delayCount = 1;
        }
    }

private:
    ResamplingProcessor<ResamplerType> inputResampler;
    ResamplingProcessor<ResamplerType> outputResampler;

    float baseFs = 48000.0f;

    DelayLine<float, DelayLineInterpolationTypes::None> delayBuffer { 16 };

    int delayCount = 0;
    bool firstPass = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResampledProcess)
};
} // namespace chowdsp
