#pragma once

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsign-conversion")
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4324) // structure was padded due to alignment specifier

namespace chowdsp
{
/**
 * FIR Filter processor that is a bit more optimized
 * than the JUCE one.
 *
 * Still, for filters with an order 512 and higher,
 * juce::dsp::Convolution is probably going to be faster.
 *
 * If fixedOrder is negative, then the order must be set at run-time.
 */
template <typename FloatType, int fixedOrder = -1, size_t maxChannelCount = defaultChannelCount>
class FIRFilter
{
public:
    /** Default constructor */
    FIRFilter();

    /** Constructs a filter with a given order */
    template <int N = fixedOrder, typename = typename std::enable_if_t<N<0>> explicit FIRFilter (int filterOrder);

    FIRFilter (FIRFilter&&) noexcept = default;
    FIRFilter& operator= (FIRFilter&&) noexcept = default;

    /**
     * Changes the order of this filter.
     *
     * Note that this will clear any coefficients which
     * had previously been loaded.
     */
    template <int N = fixedOrder>
    std::enable_if_t<(N < 0), void>
        setOrder (int newOrder);

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
        auto* z = state.data() + channel * 2 * order;
        return processSampleInternal (x, z, coefficients.data(), zPtr[channel], order, paddedOrder);
    }

    /** Process block of samples */
    void processBlock (FloatType* block, const int numSamples, const int channel = 0) noexcept
    {
        processBlock (block, block, numSamples, channel);
    }

    /** Process block of samples out-of-place */
    void processBlock (const FloatType* blockIn, FloatType* blockOut, const int numSamples, const int channel = 0) noexcept
    {
        auto* z = state.data() + channel * 2 * order;
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

        const auto inNumChannels = blockIn.getNumChannels();
        const auto numSamples = blockIn.getNumSamples();

        for (int ch = 0; ch < inNumChannels; ++ch)
            processBlock (blockIn.getReadPointer (ch), blockOut.getWritePointer (ch), numSamples, ch);
    }

    /**
     * Pushes a block of samples into the filter state without processing.
     * This can be useful for "bypassing" the filter.
     */
    void processBlockBypassed (const FloatType* block, const int numSamples, const int channel = 0) noexcept
    {
        auto* z = state.data() + channel * 2 * order;
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

    static constexpr int getPaddedOrder (int order)
    {
#if ! CHOWDSP_NO_XSIMD
        constexpr int batchSize = xsimd::batch<FloatType>::size;
        return batchSize * Math::ceiling_divide (order, batchSize);
#else
        return order;
#endif
    }

    int order = std::max (0, fixedOrder);
    int paddedOrder = getPaddedOrder (order);
    std::conditional_t<maxChannelCount == dynamicChannelCount,
                       std::vector<int>,
                       std::array<int, maxChannelCount>>
        zPtr {};

#if CHOWDSP_NO_XSIMD
    using Coeffs = std::conditional_t < fixedOrder<0,
                                                   std::vector<FloatType>,
                                                   std::array<FloatType, getPaddedOrder (fixedOrder)>>;
    Coeffs coefficients {};
#else
    using Coeffs = std::conditional_t < fixedOrder<0,
                                                   std::vector<FloatType, xsimd::default_allocator<FloatType>>,
                                                   std::array<FloatType, getPaddedOrder (fixedOrder)>>;
    alignas (SIMDUtils::defaultSIMDAlignment) Coeffs coefficients {};
#endif

    static constexpr auto heapState = maxChannelCount == dynamicChannelCount || fixedOrder < 0;
    std::conditional_t<heapState,
                       std::vector<FloatType>,
                       std::array<FloatType, maxChannelCount * 2 * fixedOrder>>
        state {};

    int numChannels = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FIRFilter)
};
} // namespace chowdsp

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
JUCE_END_IGNORE_WARNINGS_MSVC

#include "chowdsp_FIRFilter.cpp"
