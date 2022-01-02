#pragma once

namespace chowdsp
{
/**
 * Utility class for downsampling a signal by an integer ratio.
 * Note that the downsampler must be used to process block sizes
 * that are an integer multiple of the downsampling ratio.
 *
 * @tparam T            Data type to process
 * @tparam FilterOrder  Filter order to use for anti-aliasing filter (this should be an even number)
 */
template <typename T, int FilterOrder = 4>
class Downsampler
{
public:
    Downsampler() = default;

    /** Prepares the downsampler to process signal at a given upsampling ratio */
    void prepare (juce::dsp::ProcessSpec spec, int downsampleRatio)
    {
        ratio = downsampleRatio;
        downsampledBuffer.setSize ((int) spec.numChannels, (int) spec.maximumBlockSize / ratio);

        aaFilters.clear();
        aaFilters.resize (spec.numChannels);

        auto fc = T (0.995 * (spec.sampleRate * 0.5));
        auto Qs = QValCalcs::butterworth_Qs<T, FilterOrder>();
        for (size_t ch = 0; ch < spec.numChannels; ++ch)
        {
            aaFilters[ch].resize (FilterOrder / 2);
            for (int i = 0; i < FilterOrder / 2; ++i)
                aaFilters[ch][(size_t) i].calcCoefs (fc, Qs[(size_t) i], (T) spec.sampleRate);
        }

        reset();
    }

    /** Resets the downsampler state */
    void reset()
    {
        for (auto& channelFilters : aaFilters)
            for (auto& filt : channelFilters)
                filt.reset();

        downsampledBuffer.clear();
    }

    /** Returns the current downsampling ratio */
    int getDownsamplingRatio() const noexcept { return ratio; }

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

            auto y = data[n];
            for (auto& filt : aaFilters[(size_t) channel])
                y = filt.processSample (y);

            downsampledData[startSample] = y;
        }
    }

    /**
     * Process a block of data.
     * Note that the block size must be an integer multiple of the downsampling ratio.
     */
    juce::dsp::AudioBlock<T> process (const juce::dsp::AudioBlock<T>& block) noexcept
    {
        auto outBlock = juce::dsp::AudioBlock<T> { downsampledBuffer };

        const auto numChannels = block.getNumChannels();
        const auto numSamples = (int) block.getNumSamples();

        // Downsampler must be used on blocks with sizes that are integer multiples of the downsampling ratio!
        jassert (numSamples % ratio == 0);

        for (size_t ch = 0; ch < numChannels; ++ch)
            process (block.getChannelPointer (ch), outBlock.getChannelPointer (ch), (int) ch, numSamples);

        return outBlock.getSubBlock (0, size_t (numSamples / ratio));
    }

private:
    int ratio = 1;
    std::vector<std::vector<SecondOrderLPF<T>>> aaFilters; // anti-aliasing filters

    juce::AudioBuffer<T> downsampledBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Downsampler)
};
} // namespace chowdsp
