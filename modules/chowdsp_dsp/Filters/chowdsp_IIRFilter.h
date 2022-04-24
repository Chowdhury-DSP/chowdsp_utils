#pragma once

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsign-conversion")

namespace chowdsp
{
/** IIR filter of arbirtary order.
 * Uses Transposed Direct Form II:
 * https://ccrma.stanford.edu/~jos/fp/Transposed_Direct_Forms.html
 */
template <size_t order, typename FloatType = float>
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
        z.resize (1);
        reset();
    }

    IIRFilter (IIRFilter&&) noexcept = default;
    IIRFilter& operator= (IIRFilter&&) noexcept = default;

    /** Prepares the filter for processing a new number of channels */
    void prepare (int numChannels)
    {
        z.resize (numChannels);
    }

    /** Reset filter state */
    virtual void reset()
    {
        for (auto& state : z)
            std::fill (state.begin(), state.end(), 0.0f);
    }

    /** Optimized processing call for first-order filter */
    template <int N = order>
    inline std::enable_if_t<N == 1, FloatType>
        processSample (FloatType x, int ch = 0) noexcept
    {
        FloatType y = z[ch][1] + x * b[0];
        z[ch][order] = x * b[order] - y * a[order];
        return y;
    }

    /** Optimized processing call for second-order filter */
    template <int N = order>
    inline std::enable_if_t<N == 2, FloatType>
        processSample (FloatType x, int ch = 0) noexcept
    {
        FloatType y = z[ch][1] + x * b[0];
        z[ch][1] = z[ch][2] + x * b[1] - y * a[1];
        z[ch][order] = x * b[order] - y * a[order];
        return y;
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

    /** Process block of samples */
    virtual void processBlock (FloatType* block, const int numSamples, const int channel = 0) noexcept
    {
        for (int n = 0; n < numSamples; ++n)
            block[n] = processSample (block[n], channel);
    }

    /** Set coefficients to new values */
    virtual void setCoefs (const FloatType (&newB)[order + 1], const FloatType (&newA)[order + 1])
    {
        std::copy (newB, &newB[order + 1], b);
        std::copy (newA, &newA[order + 1], a);
    }

protected:
    FloatType a[order + 1];
    FloatType b[order + 1];
    std::vector<std::array<FloatType, order + 1>> z;

    template <typename PrototypeFilter>
    friend class ModFilterWrapper;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IIRFilter)
};

} // namespace chowdsp

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
