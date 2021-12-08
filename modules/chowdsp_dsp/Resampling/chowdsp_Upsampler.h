#pragma once

namespace chowdsp
{
template <typename T, int FilterOrder = 4>
class Upsampler
{
public:
    Upsampler() = default;

    void prepare (juce::dsp::ProcessSpec spec, int upsampleRatio)
    {
        ratio = upsampleRatio;
        upsampledBuffer.setSize ((int) spec.numChannels, (int) spec.maximumBlockSize * ratio);

        aiFilters.clear();
        aiFilters.resize (spec.numChannels);

        auto fc = T (0.98 * (spec.sampleRate * 0.5));
        auto Qs = QValCalcs::butterworth_Qs<T, FilterOrder>();
        for (size_t ch = 0; ch < spec.numChannels; ++ch)
        {
            aiFilters[ch].resize (FilterOrder / 2);
            for (int i = 0; i < FilterOrder / 2; ++i)
                aiFilters[ch][(size_t) i].calcCoefs (fc, Qs[(size_t) i], ratio * (T) spec.sampleRate);
        }

        reset();
    }

    void reset()
    {
        for (auto& channelFilters : aiFilters)
            for (auto& filt : channelFilters)
                filt.reset();

        upsampledBuffer.clear();
    }

    int getUpsamplingRatio() const noexcept { return ratio; }

    void process (const T* data, T* upsampledData, const int channel, const int numSamples) noexcept
    {
        for (int n = 0; n < numSamples; ++n)
        {
            int startSample = n * ratio;
            upsampledData[startSample] = data[n];

            for (int i = startSample + 1; i < startSample + ratio; ++i)
                upsampledData[i] = (T) 0;
        }

        for (auto& filt : aiFilters[(size_t) channel])
            filt.processBlock (upsampledData, numSamples * ratio);
    }

    juce::dsp::AudioBlock<T> process (const juce::dsp::AudioBlock<T>& block) noexcept
    {
        auto outBlock = juce::dsp::AudioBlock<T> { upsampledBuffer };

        const auto numChannels = block.getNumChannels();
        const auto numSamples = (int) block.getNumSamples();

        for (size_t ch = 0; ch < numChannels; ++ch)
            process (block.getChannelPointer (ch), outBlock.getChannelPointer (ch), (int) ch, numSamples);

        return outBlock;
    }

private:
    int ratio = 1;
    std::vector<std::vector<SecondOrderLPF<T>>> aiFilters; // anti-imaging filters

    juce::AudioBuffer<float> upsampledBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Upsampler)
};
} // namespace chowdsp
