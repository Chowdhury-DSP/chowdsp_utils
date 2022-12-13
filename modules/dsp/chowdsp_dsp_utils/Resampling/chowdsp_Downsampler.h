#pragma once

namespace chowdsp
{
/**
 * Utility class for downsampling a signal by an integer ratio.
 * Note that the downsampler must be used to process block sizes
 * that are an integer multiple of the downsampling ratio.
 *
 * @tparam T                        Data type to process
 * @tparam AntiAliasingFilterType   Filter type to use for an anti-aliasing filter, for example chowdsp::ButterworthFilter<8>
 */
template <typename T, typename AntiAliasingFilterType>
class Downsampler
{
public:
    Downsampler() = default;

    /** Prepares the downsampler to process signal at a given upsampling ratio */
    void prepare (juce::dsp::ProcessSpec spec, int downsampleRatio)
    {
        ratio = downsampleRatio;
        downsampledBuffer.setMaxSize ((int) spec.numChannels, (int) spec.maximumBlockSize / ratio);

        aaFilter.prepare ((int) spec.numChannels);

        auto fc = T (0.995 * ((spec.sampleRate / ratio) * 0.5));
        aaFilter.calcCoefs (fc, CoefficientCalculators::butterworthQ<T>, (T) spec.sampleRate);

        reset();
    }

    /** Resets the downsampler state */
    void reset()
    {
        aaFilter.reset();
        downsampledBuffer.clear();
    }

    /** Returns the current downsampling ratio */
    [[nodiscard]] int getDownsamplingRatio() const noexcept { return ratio; }

    /**
     * Process a single-channel block of data
     * @param data              Block of input samples.
     * @param downsampledData   Block of output samples. Must point to a block of memory of size numSamples / ratio.
     * @param channel           Channel index to process.
     * @param numSamples        Number of input samples to process. Must be an integer multiple of the ratio.
     */
    void process (const T* data, T* downsampledData, const int channel, const int numSamples) noexcept
    {
        // Downsampler must be used on blocks with sizes that are integer multiples of the downsampling ratio!
        jassert (numSamples % ratio == 0);

        for (int n = 0; n < numSamples; ++n)
        {
            int startSample = n / ratio;

            auto y = aaFilter.processSample (data[n], channel);
            downsampledData[startSample] = y;
        }
    }

    /**
     * Process a block of data.
     * Note that the block size must be an integer multiple of the downsampling ratio.
     */
    BufferView<T> process (const BufferView<const T>& block) noexcept
    {
        const auto numChannels = block.getNumChannels();
        const auto numSamples = block.getNumSamples();

        // Downsampler must be used on blocks with sizes that are integer multiples of the downsampling ratio!
        jassert (numSamples % ratio == 0);

        for (int ch = 0; ch < numChannels; ++ch)
            process (block.getReadPointer (ch), downsampledBuffer.getWritePointer (ch), ch, numSamples);

        return { downsampledBuffer, 0, numSamples / ratio };
    }

private:
    int ratio = 1;
    AntiAliasingFilterType aaFilter;

    Buffer<T> downsampledBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Downsampler)
};
} // namespace chowdsp
