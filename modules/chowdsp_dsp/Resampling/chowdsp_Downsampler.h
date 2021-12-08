#pragma once

namespace chowdsp
{
template <typename T, int FilterOrder = 4>
class Downsampler
{
public:
    Downsampler() = default;

    void prepare (juce::dsp::ProcessSpec spec, int upsampleRatio)
    {
        ratio = upsampleRatio;
        downsampledBuffer.setSize ((int) spec.numChannels, (int) spec.maximumBlockSize / ratio);

        aaFilters.clear();
        aaFilters.resize (spec.numChannels);

        auto fc = T (0.98 * (spec.sampleRate * 0.5));
        auto Qs = QValCalcs::butterworth_Qs<T, FilterOrder>();
        for (size_t ch = 0; ch < spec.numChannels; ++ch)
        {
            aaFilters[ch].resize (FilterOrder / 2);
            for (int i = 0; i < FilterOrder / 2; ++i)
                aaFilters[ch][(size_t) i].calcCoefs (fc, Qs[(size_t) i], (T) spec.sampleRate);
        }

        reset();
    }

    void reset()
    {
        for (auto& channelFilters : aaFilters)
            for (auto& filt : channelFilters)
                filt.reset();

        downsampledBuffer.clear();
    }

    int getDownsamplingRatio() const noexcept { return ratio; }

    void process (const T* data, T* downsampledData, const int channel, const int numSamples) noexcept
    {
        for (int n = 0; n < numSamples; ++n)
        {
            int startSample = n / ratio;

            auto y = data[n];
            for (auto& filt : aaFilters[(size_t) channel])
                y = filt.processSample (y);

            downsampledData[startSample] = y;
        }
    }

    juce::dsp::AudioBlock<T> process (const juce::dsp::AudioBlock<T>& block) noexcept
    {
        auto outBlock = juce::dsp::AudioBlock<T> { downsampledBuffer };

        const auto numChannels = block.getNumChannels();
        const auto numSamples = (int) block.getNumSamples();

        for (size_t ch = 0; ch < numChannels; ++ch)
            process (block.getChannelPointer (ch), outBlock.getChannelPointer (ch), (int) ch, numSamples);

        return outBlock;
    }

private:
    int ratio = 1;
    std::vector<std::vector<SecondOrderLPF<T>>> aaFilters; // anti-aliasing filters

    juce::AudioBuffer<float> downsampledBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Downsampler)
};
} // namespace chowdsp
