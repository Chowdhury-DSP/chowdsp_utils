#pragma once

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsign-conversion")

namespace chowdsp
{
/**
 * FIR Filter processor that is a bit more optimized
 * than the JUCE one.
 *
 * Still, for filters with an order 512 and higher,
 * juce::dsp::Convolution is probably going to be faster.
 */
template <typename FloatType>
class FIRFilter
{
public:
    /** Default constructor */
    FIRFilter();

    /** Constructs a filter with a given order */
    explicit FIRFilter (int filterOrder);

    FIRFilter (FIRFilter&&) noexcept = default;
    FIRFilter& operator= (FIRFilter&&) noexcept = default;

    /**
     * Changes the order of this filter.
     *
     * Note that this will clear any coefficients which
     * had previously been loaded.
     */
    void setOrder (int newOrder);

    /** Returns the current filter order */
    [[nodiscard]] int getOrder() const noexcept { return order; }

    /** Prepares the filter for processing a new number of channels */
    void prepare (int numChannels);

    /** Reset filter state */
    void reset() noexcept;

    /**
     * Copies a new set of coefficients to use for the filter.
     * The length of data pointed to by the incoming data,
     * must be exactly the same as the filter order.
     */
    void setCoefficients (const FloatType* coeffsData);

    /** Process a single sample */
    inline FloatType processSample (FloatType x, int channel = 0) noexcept
    {
        return processSampleInternal (x, state[channel].data(), coefficients.data(), zPtr[channel], order, paddedOrder);
    }

    /** Process block of samples */
    void processBlock (FloatType* block, const int numSamples, const int channel = 0) noexcept
    {
        processBlock (block, block, numSamples, channel);
    }

    /** Process block of samples out-of-place */
    void processBlock (const FloatType* blockIn, FloatType* blockOut, const int numSamples, const int channel = 0) noexcept
    {
        auto* z = state[channel].data();
        const auto* h = coefficients.data();
        ScopedValue zPtrLocal { zPtr[channel] };

        for (int n = 0; n < numSamples; ++n)
            blockOut[n] = processSampleInternal (blockIn[n], z, h, zPtrLocal.get(), order, paddedOrder);
    }

    /** Process block of samples */
    void processBlock (const BufferView<FloatType>& block) noexcept
    {
        processBlock (block, block);
    }

    /** Process block of samples out-of-place */
    void processBlock (const BufferView<const FloatType>& blockIn, const BufferView<FloatType>& blockOut) noexcept
    {
        jassert (blockIn.getNumChannels() == blockOut.getNumChannels());
        jassert (blockIn.getNumSamples() == blockOut.getNumSamples());

        const auto numChannels = blockIn.getNumChannels();
        const auto numSamples = blockIn.getNumSamples();

        for (int ch = 0; ch < numChannels; ++ch)
            processBlock (blockIn.getReadPointer (ch), blockOut.getWritePointer (ch), numSamples, ch);
    }

    /**
     * Pushes a block of samples into the filter state without processing.
     * This can be useful for "bypassing" the filter.
     */
    void processBlockBypassed (const FloatType* block, const int numSamples, const int channel = 0) noexcept
    {
        auto* z = state[channel].data();
        ScopedValue zPtrLocal { zPtr[channel] };

        for (int n = 0; n < numSamples; ++n)
            processSampleInternalBypassed (block[n], z, zPtrLocal.get(), order);
    }

    /**
     * Pushes a block of samples into the filter state without processing.
     * This can be useful for "bypassing" the filter.
     */
    void processBlockBypassed (const BufferView<const FloatType>& block) noexcept
    {
        const auto numSamples = block.getNumSamples();
        for (const auto [channel, data] : buffer_iters::channels (block))
            processBlockBypassed (data.data(), numSamples, channel);
    }

private:
    static FloatType processSampleInternal (FloatType x, FloatType* z, const FloatType* h, int& zPtr, int order, int paddedOrder) noexcept;
    static FloatType simdInnerProduct (const FloatType* z, const FloatType* h, int N);
    static void processSampleInternalBypassed (FloatType x, FloatType* z, int& zPtr, int order) noexcept;

    int order = 0;
    int paddedOrder = 0;
    std::vector<int> zPtr;

#if CHOWDSP_NO_XSIMD
    std::vector<FloatType> coefficients;
#else
    std::vector<FloatType, xsimd::default_allocator<FloatType>> coefficients;
#endif
    std::vector<std::vector<FloatType>> state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FIRFilter)
};
} // namespace chowdsp

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
