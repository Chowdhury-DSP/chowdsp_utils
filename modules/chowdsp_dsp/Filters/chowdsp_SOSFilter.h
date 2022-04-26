#pragma once

namespace chowdsp
{
/** Base class for higher-order filters made up of second-order sections */
template <int order, typename FloatType = float>
class SOSFilter
{
public:
    static_assert (order % 2 == 0, "SOSFilter currently only supports even-ordered filters");

    static constexpr auto Order = order;

    SOSFilter() = default;

    void prepare (int numChannels)
    {
        for (auto& sos : secondOrderSections)
            sos.prepare (numChannels);

        channelPointers.resize ((size_t) numChannels);
    }

    void reset()
    {
        for (auto& sos : secondOrderSections)
            sos.reset();
    }

    inline FloatType processSample (FloatType x, int channel = 0) noexcept
    {
        for (auto& sos : secondOrderSections)
            x = sos.processSample (x, channel);
        return x;
    }

    /** Process block of samples */
    void processBlock (FloatType* block, const int numSamples, const int channel = 0) noexcept
    {
        for (auto& sos : secondOrderSections)
            sos.processBlock (block, numSamples, channel);
    }

    /** Process block of samples */
    void processBlock (juce::dsp::AudioBlock<FloatType>& block) noexcept
    {
        for (auto& sos : secondOrderSections)
            sos.processBlock (block);
    }

    /** Process block of samples with a custom modulation callback which is called every sample */
    template <typename Modulator>
    void processBlockWithModulation (juce::dsp::AudioBlock<FloatType>& block, Modulator&& modulator) noexcept
    {
        const auto numChannels = block.getNumChannels();
        const auto numSamples = (int) block.getNumSamples();

        for (size_t channel = 0; channel < numChannels; ++channel)
            channelPointers[channel] = block.getChannelPointer (channel);

        for (int n = 0; n < numSamples; ++n)
        {
            modulator (n);
            for (size_t channel = 0; channel < numChannels; ++channel)
                channelPointers[channel][n] = processSample (channelPointers[channel][n], (int) channel);
        }
    }

protected:
    std::array<IIRFilter<2, FloatType>, order / 2> secondOrderSections;

private:
    std::vector<FloatType*> channelPointers;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SOSFilter)
};
}
