#pragma once

// include <Accelerate> on Apple devices so we can use vDSP_dotpr
#if JUCE_MAC || JUCE_IOS
#define Point CarbonDummyPointName
#define Component CarbonDummyCompName
#include <Accelerate/Accelerate.h>
#undef Point
#undef Component
#endif

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
    FIRFilter()
    {
        prepare (1);
    }

    /** Constructs a filter with a given order */
    explicit FIRFilter (int filterOrder) : order (filterOrder)
    {
        prepare (1);
        setOrder (filterOrder);
    }

    FIRFilter (FIRFilter&&) noexcept = default;
    FIRFilter& operator= (FIRFilter&&) noexcept = default;

    /**
     * Changes the order of this filter.
     *
     * Note that this will clear any coefficients which
     * had previously been loaded.
     */
    void setOrder (int newOrder)
    {
        order = newOrder;

        static constexpr int batchSize = xsimd::batch<FloatType>::size;
        paddedOrder = batchSize * Math::ceiling_divide (order, batchSize);
        coefficients.resize (paddedOrder, {});
        prepare ((int) state.size());
    }

    /** Returns the current filter order */
    [[nodiscard]] int getOrder() const noexcept { return order; }

    /** Prepares the filter for processing a new number of channels */
    void prepare (int numChannels)
    {
        state.resize (numChannels, std::vector<FloatType> (2 * order, {}));
        zPtr = 0;
    }

    /** Reset filter state */
    void reset()
    {
        for (auto& channelState : state)
            std::fill (channelState.begin(), channelState.end(), 0.0f);
        zPtr = 0;
    }

    /**
     * Copies a new set of coefficients to use for the filter.
     * The length of data pointed to by the incoming data,
     * must be exactly the same as the filter order.
     */
    void setCoefficients (const FloatType* coeffsData)
    {
        std::copy (coeffsData, coeffsData + order, coefficients.begin());
    }

    /** Process a single sample */
    inline FloatType processSample (FloatType x, int channel = 0) noexcept
    {
        return processSampleInternal (x, state[channel].data(), coefficients.data(), zPtr, order, paddedOrder);
    }

    /** Process block of samples */
    void processBlock (FloatType* block, const int numSamples, const int channel = 0) noexcept
    {
        auto* z = state[channel].data();
        const auto* h = coefficients.data();
        chowdsp::ScopedValue<int> zPtrLocal { zPtr };

        for (int n = 0; n < numSamples; ++n)
            block[n] = processSampleInternal (block[n], z, h, zPtrLocal.get(), order, paddedOrder);
    }

    /** Process block of samples */
    void processBlock (juce::dsp::AudioBlock<FloatType>& block) noexcept
    {
        const auto numChannels = (int) block.getNumChannels();
        const auto numSamples = (int) block.getNumSamples();
        for (int channel = 0; channel < numChannels; ++channel)
            processBlock (block.getChannelPointer (channel), numSamples, channel);
    }

private:
    static inline FloatType processSampleInternal (FloatType x, FloatType* z, const FloatType* h, int& zPtr, int order, int paddedOrder) noexcept
    {
        // insert input into double-buffered state
        z[zPtr] = x;
        z[zPtr + order] = x;

#if JUCE_MAC || JUCE_IOS
        auto y = 0.0f;
        vDSP_dotpr (z + zPtr, 1, h, 1, &y, paddedOrder); // use Acclerate inner product (if available)
#else
        auto y = simdInnerProduct (z + zPtr, h, paddedOrder); // compute inner product
#endif

        zPtr = (zPtr == 0 ? order - 1 : zPtr - 1); // iterate state pointer in reverse
        return y;
    }

    static FloatType simdInnerProduct (const FloatType* z, const FloatType* h, int N)
    {
        using b_type = xsimd::batch<FloatType>;
        static constexpr int inc = b_type::size;

        // since the size of the coefficients vector is padded, we can vectorize
        // the whole loop, and don't have to worry about leftover samples.
        jassert (N % inc == 0); // something is wrong with the padding...

        b_type batch_y {};
        for (int i = 0; i < N; i += inc)
        {
            b_type hReg = xsimd::load_aligned (&h[i]); // coefficients should always be aligned!
            b_type zReg = xsimd::load_unaligned (&z[i]); // state probably won't be aligned

            batch_y += hReg * zReg;
        }

        return xsimd::hadd (batch_y);
    }

    int order = 0;
    int paddedOrder = 0;
    int zPtr = 0;

    std::vector<FloatType, xsimd::default_allocator<FloatType>> coefficients;
    std::vector<std::vector<FloatType>> state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FIRFilter)
};
} // namespace chowdsp

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
