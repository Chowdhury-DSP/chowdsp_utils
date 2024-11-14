#pragma once

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsign-conversion")

namespace chowdsp
{
/** IIR filter of arbirtary order.
 * Uses Transposed Direct Form II:
 * https://ccrma.stanford.edu/~jos/fp/Transposed_Direct_Forms.html
 */
template <size_t order, typename FloatType = float, size_t maxChannelCount = defaultChannelCount>
class IIRFilter
{
public:
    using SampleType = FloatType;
    using NumericType = SampleTypeHelpers::NumericType<FloatType>;
    static constexpr auto Order = order;
    static constexpr bool HasQParameter = order > 1;
    static constexpr bool HasGainParameter = false;

    IIRFilter()
    {
        prepare (1);
        reset();
    }

    IIRFilter (IIRFilter&&) noexcept = default;
    IIRFilter& operator= (IIRFilter&&) noexcept = default;

    /** Prepares the filter for processing a new number of channels */
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        prepare (static_cast<int> (spec.numChannels));
    }

    /** Prepares the filter for processing a new number of channels */
    void prepare ([[maybe_unused]] int numChannels)
    {
        if constexpr (maxChannelCount == dynamicChannelCount)
            z.resize (numChannels);
        else
            jassert (numChannels <= static_cast<int> (maxChannelCount));
    }

    /** Reset filter state */
    void reset()
    {
        for (auto& state : z)
            std::fill (state.begin(), state.end(), 0.0f);
    }

    /** Optimized processing call for first-order filter */
    template <int N = order>
    inline std::enable_if_t<N == 1, FloatType>
        processSample (FloatType x, int ch = 0) noexcept
    {
        return processSample1stOrder (x, z[ch][1]);
    }

    /** Optimized processing call for second-order filter */
    template <int N = order>
    inline std::enable_if_t<N == 2, FloatType>
        processSample (FloatType x, int ch = 0) noexcept
    {
        return processSample2ndOrder (x, z[ch][1], z[ch][2]);
    }

    /** Generalized processing call for Nth-order filter */
    template <int N = order>
    inline std::enable_if_t<(N > 2), FloatType>
        processSample (FloatType x, int ch = 0) noexcept
    {
        FloatType y = z[ch][1] + x * b[0];

        for (size_t i = 1; i < order; ++i)
            z[ch][i] = z[ch][i + 1] + x * b[i] - y * a[i];

        z[ch][order] = x * b[order] - y * a[order];

        return y;
    }

    /** Process block of samples (Optimized for 1st-order filters) */
    template <int N = order>
    inline std::enable_if_t<N == 1, void>
        processBlock (FloatType* outputBlock, const FloatType* inputBlock, const int numSamples, const int channel = 0) noexcept
    {
        ScopedValue z1 { z[channel][1] };
        for (int n = 0; n < numSamples; ++n)
            outputBlock[n] = processSample1stOrder (inputBlock[n], z1.get());
    }

    /** Process block of samples (Optimized for 2nd-order filters) */
    template <int N = order>
    inline std::enable_if_t<N == 2, void>
        processBlock (FloatType* outputBlock, const FloatType* inputBlock, const int numSamples, const int channel = 0) noexcept
    {
        ScopedValue z1 { z[channel][1] };
        ScopedValue z2 { z[channel][2] };
        for (int n = 0; n < numSamples; ++n)
            outputBlock[n] = processSample2ndOrder (inputBlock[n], z1.get(), z2.get());
    }

    /** Process block of samples */
    template <int N = order>
    inline std::enable_if_t<(N > 2), void>
        processBlock (FloatType* outputBlock, const FloatType* inputBlock, const int numSamples, const int channel = 0) noexcept
    {
        for (int n = 0; n < numSamples; ++n)
            outputBlock[n] = processSample (inputBlock[n], channel);
    }

    /** Process block of samples */
    void processBlock (FloatType* block, const int numSamples, const int channel = 0) noexcept
    {
        processBlock (block, block, numSamples, channel);
    }

    /** Process block of samples */
    void processBlock (const BufferView<FloatType>& block) noexcept
    {
        const auto numSamples = block.getNumSamples();
        for (auto [channel, channelData] : buffer_iters::channels (block))
            processBlock (channelData.data(), numSamples, channel);
    }

    /** Process block of samples with a custom modulation callback which is called every sample */
    template <typename Modulator>
    void processBlockWithModulation (const BufferView<FloatType>& block, Modulator&& modulator) noexcept
    {
        const auto numChannels = (int) block.getNumChannels();
        const auto numSamples = (int) block.getNumSamples();

        auto* channelPointers = block.getArrayOfWritePointers();
        for (int n = 0; n < numSamples; ++n)
        {
            modulator (n);
            for (int channel = 0; channel < numChannels; ++channel)
                channelPointers[channel][n] = processSample (channelPointers[channel][n], channel);
        }
    }

    /** Set coefficients to new values */
    void setCoefs (const FloatType (&newB)[order + 1], const FloatType (&newA)[order + 1])
    {
        std::copy (std::begin (newB), std::end (newB), std::begin (b));
        std::copy (std::begin (newA), std::end (newA), std::begin (a));
    }

    FloatType a[order + 1]; // raw feedback "a" coefficients
    FloatType b[order + 1]; // raw feedforward "b" coefficients

    using ChannelState = std::array<FloatType, order + 1>;
    using State = std::conditional_t<maxChannelCount == dynamicChannelCount, std::vector<ChannelState>, std::array<ChannelState, maxChannelCount>>;
    State z; // filter state (per-channel)

    template <typename PrototypeFilter>
    friend class ModFilterWrapper;

    /** Process a sample with a given filter state */
    inline FloatType processSample1stOrder (const FloatType& x, FloatType& z1) noexcept
    {
        FloatType y = z1 + x * b[0];
        z1 = x * b[order] - y * a[order];
        return y;
    }

    /** Process a sample with a given filter state */
    inline FloatType processSample2ndOrder (const FloatType& x, FloatType& z1, FloatType& z2) noexcept
    {
        FloatType y = z1 + x * b[0];
        z1 = z2 + x * b[1] - y * a[1];
        z2 = x * b[order] - y * a[order];
        return y;
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IIRFilter)
};

} // namespace chowdsp

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
