#include "chowdsp_FloatVectorOperations.h"

namespace chowdsp::FloatVectorOperations
{
using namespace SIMDUtils;

#ifndef DOXYGEN
namespace detail
{
    template <typename T>
    static bool isAligned (const T* p) noexcept
    {
        return juce::dsp::SIMDRegister<T>::isSIMDAligned (p);
    }

    template <typename T>
    static T* getNextSIMDPtr (T* p) noexcept
    {
        return juce::dsp::SIMDRegister<T>::getNextSIMDAlignedPtr (p);
    }

    template <typename T, typename Op>
    T reduceFallback (const T* src, int numValues, Op&& op)
    {
        T result {};
        for (int i = 0; i < numValues; ++i)
            result = op (result, src[i]);

        return result;
    }

    template <typename T, typename Op>
    T reduceFallback (const T* src1, const T* src2, int numValues, T init, Op&& op)
    {
        for (int i = 0; i < numValues; ++i)
            init = op (init, src1[i], src2[i]);

        return init;
    }

    template <typename T, typename ScalarOp, typename VecOp>
    T reduce (const T* src, int numValues, ScalarOp&& scalarOp, VecOp&& vecOp)
    {
        constexpr auto vecSize = (int) juce::dsp::SIMDRegister<T>::size();
        auto numVecOps = numValues / vecSize;

        // Fallback: not enough operations to justify vectorizing!
        if (numVecOps < 2)
            return reduceFallback (src, numValues, scalarOp);

        // Fallback: starting pointer is not aligned!
        if (! isAligned (src))
        {
            auto* nextAlignedPtr = getNextSIMDPtr (src);
            auto diff = int (nextAlignedPtr - src);
            return scalarOp (reduceFallback (src, diff, scalarOp), reduce (nextAlignedPtr, numValues - diff, scalarOp, vecOp));
        }

        // Main loop here...
        T result {};
        while (--numVecOps >= 0)
        {
            result = vecOp (result, juce::dsp::SIMDRegister<T>::fromRawArray (src));
            src += vecSize;
        }

        // leftover values that can't be vectorized...
        auto leftoverValues = numValues % vecSize;
        if (leftoverValues > 0)
            result = scalarOp (result, reduceFallback (src, leftoverValues, scalarOp));

        return result;
    }

    template <typename T, typename ScalarOp>
    T reduce (const T* src, int numValues, ScalarOp&& scalarOp)
    {
        return reduce (src, numValues, scalarOp, scalarOp);
    }

    template <typename T, typename ScalarOp, typename VecOp>
    T reduce (const T* src1, const T* src2, int numValues, T init, ScalarOp&& scalarOp, VecOp&& vecOp)
    {
        constexpr auto vecSize = (int) juce::dsp::SIMDRegister<T>::size();
        auto numVecOps = numValues / vecSize;

        // Fallback: not enough operations to justify vectorizing!
        if (numVecOps < 2)
            return reduceFallback (src1, src2, numValues, init, scalarOp);

        // Main loop here:
        T result {};
        auto vecLoop = [&] (auto&& loadOp1, auto&& loadOp2) {
            while (--numVecOps >= 0)
            {
                result = vecOp (result, loadOp1 (src1), loadOp2 (src2));
                src1 += vecSize;
                src2 += vecSize;
            }
        };

        // define load operations
        auto loadA = [] (const T* val) { return juce::dsp::SIMDRegister<T>::fromRawArray (val); };
        auto loadU = [] (const T* val) { return loadUnaligned (val); };

        // select load operations based on data alignment
        const auto isSrc1Aligned = isAligned (src1);
        const auto isSrc2Aligned = isAligned (src2);
        if (isSrc1Aligned && isSrc2Aligned)
            vecLoop (loadA, loadA);
        else if (isSrc1Aligned)
            vecLoop (loadA, loadU);
        else if (isSrc2Aligned)
            vecLoop (loadU, loadA);
        else
            vecLoop (loadU, loadU);

        // leftover values that can't be vectorized...
        auto leftoverValues = numValues % vecSize;
        if (leftoverValues > 0)
            result = reduceFallback (src1, src2, leftoverValues, result, scalarOp);

        return result;
    }

    template <typename T, typename ScalarOp>
    T reduce (const T* src1, const T* src2, int numValues, ScalarOp&& scalarOp)
    {
        return reduce (src1, src2, numValues, scalarOp, scalarOp);
    }
} // namespace detail
#endif

bool isUsingVDSP()
{
#if JUCE_USE_VDSP_FRAMEWORK
    return true;
#else
    return false;
#endif
}

float accumulate (const float* src, int numValues) noexcept
{
#if JUCE_USE_VDSP_FRAMEWORK
    float result = 0.0f;
    vDSP_sve (src, 1, &result, (vDSP_Length) numValues);
    return result;
#else
    return detail::reduce (
        src,
        numValues,
        [] (auto prev, auto next) { return prev + next; },
        [] (auto prev, auto next) { return prev + next.sum(); });
#endif
}

double accumulate (const double* src, int numValues) noexcept
{
#if JUCE_USE_VDSP_FRAMEWORK
    double result = 0.0;
    vDSP_sveD (src, 1, &result, (vDSP_Length) numValues);
    return result;
#else
    return detail::reduce (
        src,
        numValues,
        [] (auto prev, auto next) { return prev + next; },
        [] (auto prev, auto next) { return prev + next.sum(); });
#endif
}

float innerProduct (const float* src1, const float* src2, int numValues) noexcept
{
#if JUCE_USE_VDSP_FRAMEWORK
    float result = 0.0f;
    vDSP_dotpr (src1, 1, src2, 1, &result, (vDSP_Length) numValues);
    return result;
#else
    return detail::reduce (
        src1,
        src2,
        numValues,
        0.0f,
        [] (auto prev, auto next1, auto next2) { return prev + next1 * next2; },
        [] (auto prev, auto next1, auto next2) { return prev + (next1 * next2).sum(); });
#endif
}

double innerProduct (const double* src1, const double* src2, int numValues) noexcept
{
#if JUCE_USE_VDSP_FRAMEWORK
    double result = 0.0;
    vDSP_dotprD (src1, 1, src2, 1, &result, (vDSP_Length) numValues);
    return result;
#else
    return detail::reduce (
        src1,
        src2,
        numValues,
        0.0,
        [] (auto prev, auto next1, auto next2) { return prev + next1 * next2; },
        [] (auto prev, auto next1, auto next2) { return prev + (next1 * next2).sum(); });
#endif
}
} // namespace chowdsp::FloatVectorOperations
