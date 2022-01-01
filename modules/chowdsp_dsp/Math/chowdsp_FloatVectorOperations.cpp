#include <cmath>
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
    static T* getNextAlignedPtr (T* p) noexcept
    {
        return juce::dsp::SIMDRegister<T>::getNextSIMDAlignedPtr (p);
    }

    template <typename T, typename Op>
    T reduceFallback (const T* src, int numValues, T init, Op&& op)
    {
        for (int i = 0; i < numValues; ++i)
            init = op (init, src[i]);

        return init;
    }

    template <typename T, typename Op>
    T reduceFallback (const T* src1, const T* src2, int numValues, T init, Op&& op)
    {
        for (int i = 0; i < numValues; ++i)
            init = op (init, src1[i], src2[i]);

        return init;
    }

    template <typename T, typename ScalarOp, typename VecOp, typename VecReduceOp>
    T reduce (const T* src, int numValues, T init, ScalarOp&& scalarOp, VecOp&& vecOp, VecReduceOp&& vecReduceOp)
    {
        constexpr auto vecSize = (int) juce::dsp::SIMDRegister<T>::size();
        auto numVecOps = numValues / vecSize;

        // Fallback: not enough operations to justify vectorizing!
        if (numVecOps < 2)
            return reduceFallback (src, numValues, init, scalarOp);

        // Fallback: starting pointer is not aligned!
        if (! isAligned (src))
        {
            auto* nextAlignedPtr = getNextAlignedPtr (src);
            auto diff = int (nextAlignedPtr - src);
            auto initResult = reduceFallback (src, diff, init, scalarOp);
            return reduce (nextAlignedPtr, numValues - diff, initResult, scalarOp, vecOp, vecReduceOp);
        }

        // Main loop here...
        juce::dsp::SIMDRegister<T> resultVec {};
        resultVec.set (0, init);
        while (--numVecOps >= 0)
        {
            resultVec = vecOp (resultVec, juce::dsp::SIMDRegister<T>::fromRawArray (src));
            src += vecSize;
        }

        auto result = vecReduceOp (resultVec);

        // leftover values that can't be vectorized...
        auto leftoverValues = numValues % vecSize;
        if (leftoverValues > 0)
            result = reduceFallback (src, leftoverValues, result, scalarOp);

        return result;
    }

    template <typename T, typename ScalarOp, typename VecOp>
    T reduce (const T* src, int numValues, T init, ScalarOp&& scalarOp, VecOp&& vecOp)
    {
        return reduce (src, numValues, init, scalarOp, vecOp, [] (auto val) { return val.sum(); });
    }

    template <typename T, typename ScalarOp>
    T reduce (const T* src, int numValues, T init, ScalarOp&& scalarOp)
    {
        return reduce (src, numValues, init, scalarOp, scalarOp);
    }

    template <typename T, typename ScalarOp, typename VecOp, typename VecReduceOp>
    T reduce (const T* src1, const T* src2, int numValues, T init, ScalarOp&& scalarOp, VecOp&& vecOp, VecReduceOp&& vecReduceOp)
    {
        constexpr auto vecSize = (int) juce::dsp::SIMDRegister<T>::size();
        auto numVecOps = numValues / vecSize;

        // Fallback: not enough operations to justify vectorizing!
        if (numVecOps < 2)
            return reduceFallback (src1, src2, numValues, init, scalarOp);

        // Main loop here:
        auto vecLoop = [&] (auto&& loadOp1, auto&& loadOp2) {
            juce::dsp::SIMDRegister<T> resultVec {};
            while (--numVecOps >= 0)
            {
                resultVec = vecOp (resultVec, loadOp1 (src1), loadOp2 (src2));
                src1 += vecSize;
                src2 += vecSize;
            }

            return resultVec;
        };

        // define load operations
        auto loadA = [] (const T* val) { return juce::dsp::SIMDRegister<T>::fromRawArray (val); };
        auto loadU = [] (const T* val) { return loadUnaligned (val); };

        // select load operations based on data alignment
        const auto isSrc1Aligned = isAligned (src1);
        const auto isSrc2Aligned = isAligned (src2);
        T result {};
        if (isSrc1Aligned && isSrc2Aligned)
            result = vecReduceOp (vecLoop (loadA, loadA));
        else if (isSrc1Aligned)
            result = vecReduceOp (vecLoop (loadA, loadU));
        else if (isSrc2Aligned)
            result = vecReduceOp (vecLoop (loadU, loadA));
        else
            result = vecReduceOp (vecLoop (loadU, loadU));

        // leftover values that can't be vectorized...
        auto leftoverValues = numValues % vecSize;
        if (leftoverValues > 0)
            result = reduceFallback (src1, src2, leftoverValues, result, scalarOp);

        return result;
    }

    template <typename T, typename ScalarOp, typename VecOp>
    T reduce (const T* src1, const T* src2, int numValues, T init, ScalarOp&& scalarOp, VecOp&& vecOp)
    {
        return reduce (src1, src2, numValues, init, scalarOp, vecOp, [] (auto val) { return val.sum(); });
    }

    template <typename T, typename ScalarOp>
    T reduce (const T* src1, const T* src2, int numValues, T init, ScalarOp&& scalarOp)
    {
        return reduce (src1, src2, numValues, init, scalarOp, scalarOp);
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

// @TODO: Figure out why vDSP_sve is failing unit tests in CI?

float accumulate (const float* src, int numValues) noexcept
{
#if 0 // JUCE_USE_VDSP_FRAMEWORK
    float result = 0.0f;
    vDSP_sve (src, 1, &result, (vDSP_Length) numValues);
    return result;
#else
    return detail::reduce (
        src,
        numValues,
        0.0f,
        [] (auto prev, auto next) { return prev + next; });
#endif
}

double accumulate (const double* src, int numValues) noexcept
{
#if 0 // JUCE_USE_VDSP_FRAMEWORK
    double result = 0.0;
    vDSP_sveD (src, 1, &result, (vDSP_Length) numValues);
    return result;
#else
    return detail::reduce (
        src,
        numValues,
        0.0,
        [] (auto prev, auto next) { return prev + next; });
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
        [] (auto prev, auto next1, auto next2) { return prev + next1 * next2; });
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
        [] (auto prev, auto next1, auto next2) { return prev + next1 * next2; });
#endif
}

float findAbsoluteMaximum (const float* src, int numValues) noexcept
{
#if JUCE_USE_VDSP_FRAMEWORK
    float result = 0.0f;
    vDSP_maxmgv (src, 1, &result, (vDSP_Length) numValues);
    return result;
#else
    using Vec = juce::dsp::SIMDRegister<float>;
    return detail::reduce (
        src,
        numValues,
        0.0f,
        [] (auto a, auto b) { return juce::jmax (std::abs (a), std::abs (b)); },
        [] (auto a, auto b) { return Vec::max (Vec::abs (a), Vec::abs (b)); },
        [] (auto x) { return SIMDUtils::hAbsMaxSIMD (x); });
#endif
}

double findAbsoluteMaximum (const double* src, int numValues) noexcept
{
#if JUCE_USE_VDSP_FRAMEWORK
    double result = 0.0;
    vDSP_maxmgvD (src, 1, &result, (vDSP_Length) numValues);
    return result;
#else
    using Vec = juce::dsp::SIMDRegister<double>;
    return detail::reduce (
        src,
        numValues,
        0.0,
        [] (auto a, auto b) { return juce::jmax (a, std::abs (b)); },
        [] (auto a, auto b) { return Vec::max (a, Vec::abs (b)); },
        [] (auto x) { return SIMDUtils::hMaxSIMD (x); });
#endif
}
} // namespace chowdsp::FloatVectorOperations
