#pragma once

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsign-conversion")

namespace chowdsp
{

template <typename FloatType>
class FIRFilter
{
public:
    FIRFilter()
    {
        prepare (1);
    }

    explicit FIRFilter (int filterOrder) : order (filterOrder)
    {
        coefficients.resize (order);
        prepare (1);
    }

    FIRFilter (FIRFilter&&) noexcept = default;
    FIRFilter& operator= (FIRFilter&&) noexcept = default;

    /** Changes the order of this filter */
    void setOrder (int newOrder)
    {
        order = newOrder;
        coefficients.resize (order);
        prepare (state.size());
    }

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

    void setCoefficients (const FloatType* coeffsData)
    {
        std::copy (coeffsData, coeffsData + order, coefficients.begin());
    }

    inline FloatType processSample (FloatType x, int channel = 0) noexcept
    {
        return processSampleInternal (x, state[channel].data(), coefficients.data(), zPtr, order);
    }

    /** Process block of samples */
    void processBlock (FloatType* block, const int numSamples, const int channel = 0) noexcept
    {
        auto* z = state[channel].data();
        const auto* h = coefficients.data();
        chowdsp::ScopedValue<int> zPtrLocal { zPtr };

        for (int n = 0; n < numSamples; ++n)
            block[n] = processSampleInternal (block[n], z, h, zPtrLocal.get(), order);
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
    static inline FloatType processSampleInternal (FloatType x, FloatType* z, const FloatType* h, int& zPtr, int order) noexcept
    {
        // insert input into double-buffered state
        z[zPtr] = x;
        z[zPtr + order] = x;

#if JUCE_MAC || JUCE_IOS
        auto y = 0.0f;
        vDSP_dotpr (z + zPtr, 1, h, 1, &y, order); // use Acclerate inner product (if available)
#else
        auto y = simdInnerProduct (z + zPtr, h, order); // compute inner product
#endif

        zPtr = (zPtr == 0 ? order - 1 : zPtr - 1); // iterate state pointer in reverse
        return y;
    }

    static FloatType simdInnerProduct (const FloatType* z, const FloatType* h, int N)
    {
        using b_type = xsimd::batch<FloatType>;
        static constexpr int inc = b_type::size;
        const auto vec_size = N - N % inc; // size for which the vectorization is possible

        b_type batch_y {};
        for (int i = 0; i < vec_size; i += inc)
        {
            b_type hReg = xsimd::load_aligned (&h[i]); // coefficients should always be aligned!
            b_type zReg = xsimd::load_unaligned (&z[i]); // state probably won't be aligned

            batch_y += hReg * zReg;
        }

        // Remaining part that cannot be vectorized
        FloatType y = xsimd::hadd (batch_y);
        for (int i = vec_size; i < N; ++i)
            y += h[i] * z[i];

        return y;
    }

    int order = 0;
    int zPtr = 0;

    std::vector<FloatType, xsimd::default_allocator<FloatType>> coefficients;
    std::vector<std::vector<FloatType>> state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FIRFilter)
};
} // namespace chowdsp

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
