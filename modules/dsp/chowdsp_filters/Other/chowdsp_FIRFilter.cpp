#include "chowdsp_FIRFilter.h"

#if defined(__APPLE_CPP__) || defined(__APPLE_CC__)
// include <Accelerate> on Apple devices so we can use vDSP_dotpr
#include <Accelerate/Accelerate.h>
#endif

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsign-conversion")

namespace chowdsp
{
template <typename FloatType>
FIRFilter<FloatType>::FIRFilter()
{
    prepare (1);
}

template <typename FloatType>
FIRFilter<FloatType>::FIRFilter (int filterOrder) : order (filterOrder)
{
    prepare (1);
    setOrder (filterOrder);
}

template <typename FloatType>
void FIRFilter<FloatType>::setOrder (int newOrder)
{
    order = newOrder;

    static constexpr int batchSize = xsimd::batch<FloatType>::size;
    paddedOrder = batchSize * Math::ceiling_divide (order, batchSize);
    coefficients.resize (paddedOrder, {});
    prepare ((int) state.size());
}

template <typename FloatType>
void FIRFilter<FloatType>::prepare (int numChannels)
{
    state.resize (numChannels);
    for (auto& z : state)
        z.resize (2 * order, FloatType {});

    zPtr.resize (numChannels, 0);
}

template <typename FloatType>
void FIRFilter<FloatType>::reset() noexcept
{
    for (auto& channelState : state)
        std::fill (channelState.begin(), channelState.end(), 0.0f);
    std::fill (zPtr.begin(), zPtr.end(), 0);
}

template <typename FloatType>
void FIRFilter<FloatType>::setCoefficients (const FloatType* coeffsData)
{
    std::copy (coeffsData, coeffsData + order, coefficients.begin());
}

template class FIRFilter<float>;
template class FIRFilter<double>;
} // namespace chowdsp

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
