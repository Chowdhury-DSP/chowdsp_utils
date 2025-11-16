#include "chowdsp_FIRFilter.h"

#define CHOWDSP_FIR_USE_ACCELERATE 0

#if CHOWDSP_FIR_USE_ACCELERATE && (defined(__APPLE_CPP__) || defined(__APPLE_CC__))
// include <Accelerate> on Apple devices so we can use vDSP_dotpr
#include <Accelerate/Accelerate.h>
#endif

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsign-conversion")

namespace chowdsp
{
template <typename FloatType, int fixedOrder, size_t maxChannelCount>
FIRFilter<FloatType, fixedOrder, maxChannelCount>::FIRFilter()
{
    prepare (1);
}

template <typename FloatType, int fixedOrder, size_t maxChannelCount>
template <int, typename>
FIRFilter<FloatType, fixedOrder, maxChannelCount>::FIRFilter (int filterOrder) : order (filterOrder)
{
    prepare (1);
    setOrder (filterOrder);
}

template <typename FloatType, int fixedOrder, size_t maxChannelCount>
template <int N>
std::enable_if_t<(N < 0), void> FIRFilter<FloatType, fixedOrder, maxChannelCount>::setOrder (int newOrder)
{
    order = newOrder;
    paddedOrder = getPaddedOrder (order);

    coefficients.resize (paddedOrder, {});
    prepare (numChannels);
}

template <typename FloatType, int fixedOrder, size_t maxChannelCount>
void FIRFilter<FloatType, fixedOrder, maxChannelCount>::prepare (int newNumChannels)
{
    numChannels = newNumChannels;

    if constexpr (heapState)
        state.resize (numChannels * (2 * paddedOrder));

    if constexpr (maxChannelCount == dynamicChannelCount)
        zPtr.resize (numChannels, 0);

    reset();
}

template <typename FloatType, int fixedOrder, size_t maxChannelCount>
void FIRFilter<FloatType, fixedOrder, maxChannelCount>::reset() noexcept
{
    std::fill (state.begin(), state.end(), 0.0f);
    std::fill (zPtr.begin(), zPtr.end(), 0);
}

template <typename FloatType, int fixedOrder, size_t maxChannelCount>
void FIRFilter<FloatType, fixedOrder, maxChannelCount>::setCoefficients (const FloatType* coeffsData)
{
    std::copy (coeffsData, coeffsData + order, coefficients.begin());
}

template <typename FloatType, int fixedOrder, size_t maxChannelCount>
inline FloatType FIRFilter<FloatType, fixedOrder, maxChannelCount>::processSampleInternal (FloatType x, FloatType* z, const FloatType* h, int& zPtr, int order, int paddedOrder) noexcept
{
    // insert input into double-buffered state
    z[zPtr] = x;
    z[zPtr + order] = x;

#if CHOWDSP_FIR_USE_ACCELERATE && (JUCE_MAC || JUCE_IOS)
    auto y = (FloatType) 0;

    // use Acclerate inner product (if available)
    if constexpr (std::is_same_v<FloatType, float>)
        vDSP_dotpr (z + zPtr, 1, h, 1, &y, paddedOrder);
    else
        vDSP_dotprD (z + zPtr, 1, h, 1, &y, paddedOrder);
#else
    auto y = simdInnerProduct (z + zPtr, h, paddedOrder); // compute inner product
#endif

    zPtr = (zPtr == 0 ? order - 1 : zPtr - 1); // iterate state pointer in reverse
    return y;
}

#if ! CHOWDSP_NO_XSIMD
template <typename FloatType, int fixedOrder, size_t maxChannelCount>
inline FloatType FIRFilter<FloatType, fixedOrder, maxChannelCount>::simdInnerProduct (const FloatType* z, const FloatType* h, int N)
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

    return xsimd::reduce_add (batch_y);
}
#else
template <typename FloatType, int fixedOrder, size_t maxChannelCount>
inline FloatType FIRFilter<FloatType, fixedOrder, maxChannelCount>::simdInnerProduct (const FloatType* z, const FloatType* h, int N)
{
    return std::inner_product (z, z + N, h, FloatType {});
}
#endif

template <typename FloatType, int fixedOrder, size_t maxChannelCount>
inline void FIRFilter<FloatType, fixedOrder, maxChannelCount>::processSampleInternalBypassed (FloatType x, FloatType* z, int& zPtr, int order) noexcept
{
    // insert input into double-buffered state
    z[zPtr] = x;
    z[zPtr + order] = x;
    zPtr = (zPtr == 0 ? order - 1 : zPtr - 1); // iterate state pointer in reverse
}
} // namespace chowdsp

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
