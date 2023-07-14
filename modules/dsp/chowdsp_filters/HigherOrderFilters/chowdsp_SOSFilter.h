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

    /** Prepares the filter to process a new stream of audio */
    void prepare (int numChannels)
    {
        for (auto& sos : secondOrderSections)
            sos.prepare (numChannels);
    }

    /** Resets the filter state */
    void reset()
    {
        for (auto& sos : secondOrderSections)
            sos.reset();
    }

    /** Process a single sample */
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
    void processBlock (const FloatType* inputBlock, FloatType* outputBlock, const int numSamples, const int channel = 0) noexcept
    {
        secondOrderSections[0].processBlock (outputBlock, inputBlock, numSamples, channel);
        for (size_t i = 1; i < secondOrderSections.size(); ++i)
            secondOrderSections[i].processBlock (outputBlock, numSamples, channel);
    }

    /** Process block of samples */
    void processBlock (const BufferView<FloatType>& block) noexcept
    {
        for (auto& sos : secondOrderSections)
            sos.processBlock (block);
    }

    /** Process block of samples with a custom modulation callback which is called every sample */
    template <typename Modulator>
    void processBlockWithModulation (const BufferView<FloatType>& block, Modulator&& modulator) noexcept
    {
        const auto numChannels = block.getNumChannels();
        const auto numSamples = block.getNumSamples();

        auto* channelPointers = block.getArrayOfWritePointers();
        for (int n = 0; n < numSamples; ++n)
        {
            modulator (n);
            for (int channel = 0; channel < numChannels; ++channel)
                channelPointers[channel][n] = processSample (channelPointers[channel][n], channel);
        }
    }

protected:
    std::array<IIRFilter<2, FloatType>, (size_t) order / 2> secondOrderSections {};

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SOSFilter)
};
} // namespace chowdsp
