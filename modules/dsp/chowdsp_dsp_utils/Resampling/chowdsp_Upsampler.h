#pragma once

namespace chowdsp
{
/**
 * Utility class for upsampling a signal by an integer ratio
 *
 * @tparam T                        Data type to process
 * @tparam AntiImagingFilterType    Filter type to use for an anti-imaging filter, for example chowdsp::ButterworthFilter<8>
 */
template <typename T, typename AntiImagingFilterType>
class Upsampler
{
public:
    Upsampler() = default;

    /** Prepares the upsampler to process signal at a given upsampling ratio */
    void prepare (juce::dsp::ProcessSpec spec, int upsampleRatio)
    {
        ratio = upsampleRatio;
        upsampledBuffer.setMaxSize ((int) spec.numChannels, (int) spec.maximumBlockSize * ratio);

        aiFilter.prepare ((int) spec.numChannels);

        auto fc = T (0.995 * (spec.sampleRate * 0.5));
        aiFilter.calcCoefs (fc, CoefficientCalculators::butterworthQ<T>, ratio * (T) spec.sampleRate);

        reset();
    }

    /** Resets the upsampler state */
    void reset()
    {
        aiFilter.reset();
        upsampledBuffer.clear();
    }

    /** Returns the current upsampling ratio */
    [[nodiscard]] int getUpsamplingRatio() const noexcept { return ratio; }

    /**
     * Process a single-channel block of data
     * @param data              Block of input samples
     * @param upsampledData     Block of output samples. Must point to a block of memory of size numSamples * ratio
     * @param channel           Channel index to process
     * @param numSamples        Number of input samples to process
     */
    void process (const T* data, T* upsampledData, const int channel, const int numSamples) noexcept
    {
        for (int n = 0; n < numSamples; ++n)
        {
            int startSample = n * ratio;
            upsampledData[startSample] = data[n];

            for (int i = startSample + 1; i < startSample + ratio; ++i)
                upsampledData[i] = (T) 0;
        }

        aiFilter.processBlock (upsampledData, numSamples * ratio, channel);
        juce::FloatVectorOperations::multiply (upsampledData, (T) ratio, numSamples * ratio);
    }

    /** Process a block of data */
    BufferView<T> process (const BufferView<const T>& block) noexcept
    {
        const auto numChannels = block.getNumChannels();
        const auto numSamples = block.getNumSamples();

        for (int ch = 0; ch < numChannels; ++ch)
            process (block.getReadPointer (ch), upsampledBuffer.getWritePointer (ch), ch, numSamples);

        return { upsampledBuffer, 0, numSamples * ratio };
    }

private:
    int ratio = 1;
    AntiImagingFilterType aiFilter; // anti-imaging filter

    Buffer<T> upsampledBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Upsampler)
};
} // namespace chowdsp
