#pragma once

namespace chowdsp
{
/** Processor for up/downsampling a signal by a non-integer factor */
template <typename ResamplerType>
class ResampledProcess
{
public:
    static_assert (std::is_base_of_v<ResamplingTypes::BaseResampler, ResamplerType>, "ResamplerType must be derived from BaseResampler");

    ResampledProcess() = default;

    /** Move constructor */
    ResampledProcess (ResampledProcess&&) noexcept = default;

    /** Move assignment operator */
    ResampledProcess& operator= (ResampledProcess&&) noexcept = default;

    /** Prepares the resampler to process a new stream of data */
    void prepare (const juce::dsp::ProcessSpec& spec, double startRatio = 1.0)
    {
        inputResampler.prepare (spec, startRatio);
        outputResampler.prepare (spec, 1.0 / startRatio);

        leftoverBuffer.resize (spec.numChannels, 0.0f);
        leftoverAvailable = false;

        baseFs = (float) spec.sampleRate;
    }

    /** Prepares the resampler to process a new stream of data at a given target sample rate */
    void prepareWithTargetSampleRate (const juce::dsp::ProcessSpec& spec, double targetSampleRate)
    {
        prepare (spec, targetSampleRate / spec.sampleRate);
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
    [[nodiscard]] float getResampleRatio() const noexcept
    {
        return inputResampler.getResampleRatio();
    }

    /** Returns the target sample rate */
    [[nodiscard]] float getTargetSampleRate() const noexcept
    {
        return getResampleRatio() * baseFs;
    }

    /** Processes an input block of samples
     *
     *  @return the output block of generated samples at the target sample rate
     */
    BufferView<float> processIn (const BufferView<const float>& block) noexcept
    {
        return inputResampler.process (block);
    }

    void processOut (const BufferView<const float>& inBlock, const BufferView<float>& outputBlock)
    {
        auto outBlockTemp = outputResampler.process (inBlock);

        const auto availableSamples = outBlockTemp.getNumSamples();
        auto expectedSamples = outputBlock.getNumSamples();
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
            for (int ch = 0; ch < outputBlock.getNumChannels(); ++ch)
            {
                auto* destData = outputBlock.getWritePointer (ch);
                destData[0] = leftoverBuffer[(size_t) ch];
            }

            destStart = 1;
            expectedSamples -= 1;
            leftoverAvailable = false;
        }

        // do we have exactly the right number of samples to fill the output buffer?
        if (availableSamples == expectedSamples)
        {
            for (int ch = 0; ch < outputBlock.getNumChannels(); ++ch)
            {
                const auto* srcData = outBlockTemp.getReadPointer (ch);
                auto* destData = outputBlock.getWritePointer (ch);
                juce::FloatVectorOperations::copy (destData + destStart, srcData, availableSamples);
            }
            return;
        }

        // we have some extra samples after the buffer is filled!
        if (availableSamples > expectedSamples)
        {
            for (int ch = 0; ch < outputBlock.getNumChannels(); ++ch)
            {
                const auto* srcData = outBlockTemp.getReadPointer (ch);
                auto* destData = outputBlock.getWritePointer (ch);
                juce::FloatVectorOperations::copy (destData + destStart, srcData, expectedSamples);

                leftoverBuffer[(size_t) ch] = srcData[availableSamples - 1];
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
