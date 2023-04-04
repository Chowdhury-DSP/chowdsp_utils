#include <cmath>
#include "chowdsp_FloatVectorOperations.h"

namespace chowdsp::FloatVectorOperations
{
#if ! CHOWDSP_NO_XSIMD
#ifndef DOXYGEN
namespace detail
{
    template <typename T, typename Op>
    void unaryOpFallback (T* dest, const T* src, int numValues, Op&& op)
    {
        for (int i = 0; i < numValues; ++i)
            dest[i] = op (src[i]);
    }

    template <typename T, typename ScalarOp, typename VecOp, typename LoadOpType, typename StoreOpType>
    void unaryOp (T* dest, const T* src, int numValues, ScalarOp&& scalarOp, VecOp&& vecOp, LoadOpType&& loadOp, StoreOpType&& storeOp)
    {
        constexpr auto vecSize = (int) xsimd::batch<T>::size;
        auto numVecOps = numValues / vecSize;

        // Fallback: not enough operations to justify vectorizing!
        if (numVecOps < 2)
        {
            unaryOpFallback (dest, src, numValues, std::forward<ScalarOp> (scalarOp));
            return;
        }

        // Main loop here...
        while (--numVecOps >= 0)
        {
            storeOp (dest, vecOp (loadOp (src)));
            dest += vecSize;
            src += vecSize;
        }

        // leftover values that can't be vectorized...
        auto leftoverValues = numValues % vecSize;
        if (leftoverValues > 0)
            unaryOpFallback (dest, src, leftoverValues, std::forward<ScalarOp> (scalarOp));
    }

    template <typename T, typename ScalarOp, typename VecOp>
    void unaryOp (T* dest, const T* src, int numValues, ScalarOp&& scalarOp, VecOp&& vecOp)
    {
        auto loadA = [] (const auto* ptr)
        { return xsimd::load_aligned (ptr); };

        auto loadU = [] (const auto* ptr)
        { return xsimd::load_unaligned (ptr); };

        auto storeA = [] (auto* ptr, const auto& reg)
        { xsimd::store_aligned (ptr, reg); };

        auto storeU = [] (auto* ptr, const auto& reg)
        { xsimd::store_unaligned (ptr, reg); };

        if (SIMDUtils::isAligned (dest))
        {
            if (SIMDUtils::isAligned (src))
                unaryOp (dest, src, numValues, std::forward<ScalarOp> (scalarOp), std::forward<VecOp> (vecOp), loadA, storeA);
            else
                unaryOp (dest, src, numValues, std::forward<ScalarOp> (scalarOp), std::forward<VecOp> (vecOp), loadU, storeA);
        }
        else
        {
            if (SIMDUtils::isAligned (src))
                unaryOp (dest, src, numValues, std::forward<ScalarOp> (scalarOp), std::forward<VecOp> (vecOp), loadA, storeU);
            else
                unaryOp (dest, src, numValues, std::forward<ScalarOp> (scalarOp), std::forward<VecOp> (vecOp), loadU, storeU);
        }
    }

    template <typename T, typename Op>
    void unaryOp (T* dest, const T* src, int numValues, Op&& op)
    {
        unaryOp (dest, src, numValues, std::forward<Op> (op), std::forward<Op> (op));
    }

    template <typename T, typename Op>
    void binaryOpFallback (T* dest, const T* src1, const T* src2, int numValues, Op&& op)
    {
        for (int i = 0; i < numValues; ++i)
            dest[i] = op (src1[i], src2[i]);
    }

    template <typename T, typename ScalarOp, typename VecOp, typename LoadOp1Type, typename LoadOp2Type, typename StoreOpType>
    void binaryOp (T* dest, const T* src1, const T* src2, int numValues, ScalarOp&& scalarOp, VecOp&& vecOp, LoadOp1Type&& loadOp1, LoadOp2Type&& loadOp2, StoreOpType&& storeOp) // NOSONAR (too many parameters)
    {
        constexpr auto vecSize = (int) xsimd::batch<T>::size;
        auto numVecOps = numValues / vecSize;

        // Fallback: not enough operations to justify vectorizing!
        if (numVecOps < 2)
        {
            binaryOpFallback (dest, src1, src2, numValues, std::forward<ScalarOp> (scalarOp));
            return;
        }

        // Main loop here...
        while (--numVecOps >= 0)
        {
            storeOp (dest, vecOp (loadOp1 (src1), loadOp2 (src2)));
            dest += vecSize;
            src1 += vecSize;
            src2 += vecSize;
        }

        // leftover values that can't be vectorized...
        auto leftoverValues = numValues % vecSize;
        if (leftoverValues > 0)
            binaryOpFallback (dest, src1, src2, leftoverValues, std::forward<ScalarOp> (scalarOp));
    }

    template <typename T, typename ScalarOp, typename VecOp>
    void binaryOp (T* dest, const T* src1, const T* src2, int numValues, ScalarOp&& scalarOp, VecOp&& vecOp)
    {
        auto loadA = [] (const auto* ptr)
        { return xsimd::load_aligned (ptr); };

        auto loadU = [] (const auto* ptr)
        { return xsimd::load_unaligned (ptr); };

        auto storeA = [] (auto* ptr, const auto& reg)
        { xsimd::store_aligned (ptr, reg); };

        auto storeU = [] (auto* ptr, const auto& reg)
        { xsimd::store_unaligned (ptr, reg); };

        if (SIMDUtils::isAligned (dest))
        {
            if (SIMDUtils::isAligned (src1))
            {
                if (SIMDUtils::isAligned (src2))
                    binaryOp (dest, src1, src2, numValues, std::forward<ScalarOp> (scalarOp), std::forward<VecOp> (vecOp), loadA, loadA, storeA);
                else
                    binaryOp (dest, src1, src2, numValues, std::forward<ScalarOp> (scalarOp), std::forward<VecOp> (vecOp), loadA, loadU, storeA);
            }
            else
            {
                if (SIMDUtils::isAligned (src2))
                    binaryOp (dest, src1, src2, numValues, std::forward<ScalarOp> (scalarOp), std::forward<VecOp> (vecOp), loadU, loadA, storeA);
                else
                    binaryOp (dest, src1, src2, numValues, std::forward<ScalarOp> (scalarOp), std::forward<VecOp> (vecOp), loadU, loadU, storeA);
            }
        }
        else
        {
            if (SIMDUtils::isAligned (src1))
            {
                if (SIMDUtils::isAligned (src2))
                    binaryOp (dest, src1, src2, numValues, std::forward<ScalarOp> (scalarOp), std::forward<VecOp> (vecOp), loadA, loadA, storeU);
                else
                    binaryOp (dest, src1, src2, numValues, std::forward<ScalarOp> (scalarOp), std::forward<VecOp> (vecOp), loadA, loadU, storeU);
            }
            else
            {
                if (SIMDUtils::isAligned (src2))
                    binaryOp (dest, src1, src2, numValues, std::forward<ScalarOp> (scalarOp), std::forward<VecOp> (vecOp), loadU, loadA, storeU);
                else
                    binaryOp (dest, src1, src2, numValues, std::forward<ScalarOp> (scalarOp), std::forward<VecOp> (vecOp), loadU, loadU, storeU);
            }
        }
    }

    template <typename T, typename Op>
    void binaryOp (T* dest, const T* src1, const T* src2, int numValues, Op&& op)
    {
        binaryOp (dest, src1, src2, numValues, std::forward<Op> (op), std::forward<Op> (op));
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
        constexpr auto vecSize = (int) xsimd::batch<T>::size;
        auto numVecOps = numValues / vecSize;

        // Fallback: not enough operations to justify vectorizing!
        if (numVecOps < 2)
            return reduceFallback (src, numValues, init, std::forward<ScalarOp> (scalarOp));

        // Fallback: starting pointer is not aligned!
        if (! SIMDUtils::isAligned (src))
        {
            auto* nextAlignedPtr = SIMDUtils::getNextAlignedPtr (src);
            auto diff = int (nextAlignedPtr - src);
            auto initResult = reduceFallback (src, diff, init, std::forward<ScalarOp> (scalarOp));
            return reduce (nextAlignedPtr, numValues - diff, initResult, std::forward<ScalarOp> (scalarOp), std::forward<VecOp> (vecOp), std::forward<VecReduceOp> (vecReduceOp));
        }

        // Main loop here...
        T initData alignas (xsimd::default_arch::alignment())[(size_t) vecSize] {};
        initData[0] = init;
        auto resultVec = xsimd::load_aligned (initData);
        while (--numVecOps >= 0)
        {
            resultVec = vecOp (resultVec, xsimd::load_aligned (src));
            src += vecSize;
        }

        auto result = vecReduceOp (resultVec);

        // leftover values that can't be vectorized...
        auto leftoverValues = numValues % vecSize;
        if (leftoverValues > 0)
            result = reduceFallback (src, leftoverValues, result, std::forward<ScalarOp> (scalarOp));

        return result;
    }

    template <typename T, typename ScalarOp, typename VecOp>
    T reduce (const T* src, int numValues, T init, ScalarOp&& scalarOp, VecOp&& vecOp)
    {
        return reduce (src, numValues, init, std::forward<ScalarOp> (scalarOp), std::forward<VecOp> (vecOp), [] (auto val)
                       { return xsimd::reduce_add (val); });
    }

    template <typename T, typename Op>
    T reduce (const T* src, int numValues, T init, Op&& op)
    {
        return reduce (src, numValues, init, std::forward<Op> (op), std::forward<Op> (op));
    }

    template <typename T, typename ScalarOp, typename VecOp, typename VecReduceOp>
    T reduce (const T* src1, const T* src2, int numValues, T init, ScalarOp&& scalarOp, VecOp&& vecOp, VecReduceOp&& vecReduceOp)
    {
        constexpr auto vecSize = (int) xsimd::batch<T>::size;
        auto numVecOps = numValues / vecSize;

        // Fallback: not enough operations to justify vectorizing!
        if (numVecOps < 2)
            return reduceFallback (src1, src2, numValues, init, std::forward<ScalarOp> (scalarOp));

        // Main loop here:
        auto vecLoop = [&] (auto&& loadOp1, auto&& loadOp2)
        {
            xsimd::batch<T> resultVec {};
            while (--numVecOps >= 0)
            {
                resultVec = vecOp (resultVec, loadOp1 (src1), loadOp2 (src2));
                src1 += vecSize;
                src2 += vecSize;
            }

            return resultVec;
        };

        // define load operations
        auto loadA = [] (const T* val)
        { return xsimd::load_aligned (val); };
        auto loadU = [] (const T* val)
        { return xsimd::load_unaligned (val); };

        // select load operations based on data alignment
        const auto isSrc1Aligned = SIMDUtils::isAligned (src1);
        const auto isSrc2Aligned = SIMDUtils::isAligned (src2);
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
            result = reduceFallback (src1, src2, leftoverValues, result, std::forward<ScalarOp> (scalarOp));

        return result;
    }

    template <typename T, typename ScalarOp, typename VecOp>
    T reduce (const T* src1, const T* src2, int numValues, T init, ScalarOp&& scalarOp, VecOp&& vecOp)
    {
        return reduce (src1, src2, numValues, init, std::forward<ScalarOp> (scalarOp), std::forward<VecOp> (vecOp), [] (auto val)
                       { return xsimd::reduce_add (val); });
    }

    template <typename T, typename Op>
    T reduce (const T* src1, const T* src2, int numValues, T init, Op&& op)
    {
        return reduce (src1, src2, numValues, init, std::forward<Op> (op), std::forward<Op> (op));
    }
} // namespace detail
#endif // DOXYGEN
#endif // ! CHOWDSP_NO_XSIMD

bool isUsingVDSP()
{
#if JUCE_USE_VDSP_FRAMEWORK
    return true;
#else
    return false;
#endif
}

void divide (float* dest, const float* dividend, const float* divisor, int numValues) noexcept
{
#if JUCE_USE_VDSP_FRAMEWORK
    vDSP_vdiv (divisor, 1, dividend, 1, dest, 1, (vDSP_Length) numValues);
#elif CHOWDSP_NO_XSIMD
    std::transform (dividend, dividend + numValues, divisor, dest, [] (auto a, auto b)
                    { return a / b; });
#else
    detail::binaryOp (dest,
                      dividend,
                      divisor,
                      numValues,
                      [] (auto num, auto den)
                      {
                          return num / den;
                      });
#endif
}

void divide (double* dest, const double* dividend, const double* divisor, int numValues) noexcept
{
#if JUCE_USE_VDSP_FRAMEWORK
    vDSP_vdivD (divisor, 1, dividend, 1, dest, 1, (vDSP_Length) numValues);
#elif CHOWDSP_NO_XSIMD
    std::transform (dividend, dividend + numValues, divisor, dest, [] (auto a, auto b)
                    { return a / b; });
#else
    detail::binaryOp (dest,
                      dividend,
                      divisor,
                      numValues,
                      [] (auto num, auto den)
                      {
                          return num / den;
                      });
#endif
}

void divide (float* dest, float dividend, const float* divisor, int numValues) noexcept
{
#if JUCE_USE_VDSP_FRAMEWORK
    vDSP_svdiv (&dividend, divisor, 1, dest, 1, (vDSP_Length) numValues);
#elif CHOWDSP_NO_XSIMD
    std::transform (divisor, divisor + numValues, dest, [dividend] (auto x)
                    { return dividend / x; });
#else
    detail::unaryOp (dest,
                     divisor,
                     numValues,
                     [dividend] (auto x)
                     {
                         return dividend / x;
                     });
#endif
}

void divide (double* dest, double dividend, const double* divisor, int numValues) noexcept
{
#if JUCE_USE_VDSP_FRAMEWORK
    vDSP_svdivD (&dividend, divisor, 1, dest, 1, (vDSP_Length) numValues);
#elif CHOWDSP_NO_XSIMD
    std::transform (divisor, divisor + numValues, dest, [dividend] (auto x)
                    { return dividend / x; });
#else
    detail::unaryOp (dest,
                     divisor,
                     numValues,
                     [dividend] (auto x)
                     {
                         return dividend / x;
                     });
#endif
}

float accumulate (const float* src, int numValues) noexcept
{
#if JUCE_USE_VDSP_FRAMEWORK
    float result = 0.0f;
    vDSP_sve (src, 1, &result, (vDSP_Length) numValues);
    return result;
#elif CHOWDSP_NO_XSIMD
    return std::accumulate (src, src + numValues, 0.0f);
#else
    return detail::reduce (
        src,
        numValues,
        0.0f,
        [] (auto prev, auto next)
        { return prev + next; });
#endif
}

double accumulate (const double* src, int numValues) noexcept
{
#if JUCE_USE_VDSP_FRAMEWORK
    double result = 0.0;
    vDSP_sveD (src, 1, &result, (vDSP_Length) numValues);
    return result;
#elif CHOWDSP_NO_XSIMD
    return std::accumulate (src, src + numValues, 0.0);
#else
    return detail::reduce (
        src,
        numValues,
        0.0,
        [] (auto prev, auto next)
        { return prev + next; });
#endif
}

float innerProduct (const float* src1, const float* src2, int numValues) noexcept
{
#if JUCE_USE_VDSP_FRAMEWORK
    float result = 0.0f;
    vDSP_dotpr (src1, 1, src2, 1, &result, (vDSP_Length) numValues);
    return result;
#elif CHOWDSP_NO_XSIMD
    return std::inner_product (src1, src1 + numValues, src2, 0.0f);
#else
    return detail::reduce (
        src1,
        src2,
        numValues,
        0.0f,
        [] (auto prev, auto next1, auto next2)
        { return prev + next1 * next2; });
#endif
}

double innerProduct (const double* src1, const double* src2, int numValues) noexcept
{
#if JUCE_USE_VDSP_FRAMEWORK
    double result = 0.0;
    vDSP_dotprD (src1, 1, src2, 1, &result, (vDSP_Length) numValues);
    return result;
#elif CHOWDSP_NO_XSIMD
    return std::inner_product (src1, src1 + numValues, src2, 0.0);
#else
    return detail::reduce (
        src1,
        src2,
        numValues,
        0.0,
        [] (auto prev, auto next1, auto next2)
        { return prev + next1 * next2; });
#endif
}

float findAbsoluteMaximum (const float* src, int numValues) noexcept
{
#if JUCE_USE_VDSP_FRAMEWORK
    float result = 0.0f;
    vDSP_maxmgv (src, 1, &result, (vDSP_Length) numValues);
    return result;
#elif CHOWDSP_NO_XSIMD
    return [] (const auto& begin, const auto end)
    { return std::abs (*std::max_element (begin, end, [] (auto a, auto b)
                                          { return std::abs (a) < std::abs (b); })); }(src, src + numValues);
#else
    return detail::reduce (
        src,
        numValues,
        0.0f,
        [] (auto a, auto b)
        { return juce::jmax (std::abs (a), std::abs (b)); },
        [] (auto a, auto b)
        { return xsimd::max (xsimd::abs (a), xsimd::abs (b)); },
        [] (auto x)
        { return SIMDUtils::hAbsMaxSIMD (x); });
#endif
}

double findAbsoluteMaximum (const double* src, int numValues) noexcept
{
#if JUCE_USE_VDSP_FRAMEWORK
    double result = 0.0;
    vDSP_maxmgvD (src, 1, &result, (vDSP_Length) numValues);
    return result;
#elif CHOWDSP_NO_XSIMD
    return [] (const auto& begin, const auto end)
    { return std::abs (*std::max_element (begin, end, [] (auto a, auto b)
                                          { return std::abs (a) < std::abs (b); })); }(src, src + numValues);
#else
    return detail::reduce (
        src,
        numValues,
        0.0,
        [] (auto a, auto b)
        { return juce::jmax (a, std::abs (b)); },
        [] (auto a, auto b)
        { return xsimd::max (a, xsimd::abs (b)); },
        [] (auto x)
        { return SIMDUtils::hMaxSIMD (x); });
#endif
}

#if ! CHOWDSP_NO_XSIMD
template <typename T>
void integerPowerT (T* dest, const T* src, int exponent, int numValues) noexcept
{
    // negative values are not supported!
    jassert (exponent >= 0);

    using Power::ipow;
    switch (exponent)
    {
        case 0:
            juce::FloatVectorOperations::fill (dest, (T) 1, numValues);
            break;
        case 1:
            juce::FloatVectorOperations::copy (dest, src, numValues);
            break;
        case 2:
            juce::FloatVectorOperations::multiply (dest, src, src, numValues);
            break;
        case 3:
            detail::unaryOp (dest, src, numValues, [] (auto x)
                             { return ipow<3> (x); });
            break;
        case 4:
            detail::unaryOp (dest, src, numValues, [] (auto x)
                             { return ipow<4> (x); });
            break;
        case 5:
            detail::unaryOp (dest, src, numValues, [] (auto x)
                             { return ipow<5> (x); });
            break;
        case 6:
            detail::unaryOp (dest, src, numValues, [] (auto x)
                             { return ipow<6> (x); });
            break;
        case 7:
            detail::unaryOp (dest, src, numValues, [] (auto x)
                             { return ipow<7> (x); });
            break;
        case 8:
            detail::unaryOp (dest, src, numValues, [] (auto x)
                             { return ipow<8> (x); });
            break;
        case 9:
            detail::unaryOp (dest, src, numValues, [] (auto x)
                             { return ipow<9> (x); });
            break;
        case 10:
            detail::unaryOp (dest, src, numValues, [] (auto x)
                             { return ipow<10> (x); });
            break;
        case 11:
            detail::unaryOp (dest, src, numValues, [] (auto x)
                             { return ipow<11> (x); });
            break;
        case 12:
            detail::unaryOp (dest, src, numValues, [] (auto x)
                             { return ipow<12> (x); });
            break;
        case 13:
            detail::unaryOp (dest, src, numValues, [] (auto x)
                             { return ipow<13> (x); });
            break;
        case 14:
            detail::unaryOp (dest, src, numValues, [] (auto x)
                             { return ipow<14> (x); });
            break;
        case 15:
            detail::unaryOp (dest, src, numValues, [] (auto x)
                             { return ipow<15> (x); });
            break;
        case 16:
            detail::unaryOp (dest, src, numValues, [] (auto x)
                             { return ipow<16> (x); });
            break;
        default:
            // this method will not be as fast for values outside the range [0, 16]
            detail::unaryOp (
                dest,
                src,
                numValues,
                [exponent] (auto x)
                { return std::pow (x, (T) exponent); },
                [exponent] (auto x)
                { return xsimd::pow (x, xsimd::batch<T> ((T) exponent)); });
            break;
    }
}
#endif // ! CHOWDSP_NO_XSIMD

void integerPower (float* dest, const float* src, int exponent, int numValues) noexcept
{
#if CHOWDSP_NO_XSIMD
    for (int i = 0; i < numValues; ++i)
        dest[i] = std::pow (src[i], (float) exponent);
#else
    integerPowerT (dest, src, exponent, numValues);
#endif
}

void integerPower (double* dest, const double* src, int exponent, int numValues) noexcept
{
#if CHOWDSP_NO_XSIMD
    for (int i = 0; i < numValues; ++i)
        dest[i] = std::pow (src[i], (double) exponent);
#else
    integerPowerT (dest, src, exponent, numValues);
#endif
}

float computeRMS (const float* src, int numValues) noexcept
{
#if JUCE_USE_VDSP_FRAMEWORK
    float result = 0.0f;
    vDSP_rmsqv (src, 1, &result, (vDSP_Length) numValues);
    return result;
#elif CHOWDSP_NO_XSIMD
    return [] (const float* data, int numSamples)
    {
        auto squareSum = 0.0;
        for (int i = 0; i < numSamples; ++i)
            squareSum += data[i] * data[i];
        return std::sqrt (squareSum / (float) numSamples);
    }(src, numValues);
#else
    const auto squareSum = detail::reduce (src,
                                           numValues,
                                           0.0f,
                                           [] (auto prev, auto next)
                                           { return prev + next * next; });
    return std::sqrt (squareSum / (float) numValues);
#endif
}

double computeRMS (const double* src, int numValues) noexcept
{
#if JUCE_USE_VDSP_FRAMEWORK
    double result = 0.0;
    vDSP_rmsqvD (src, 1, &result, (vDSP_Length) numValues);
    return result;
#elif CHOWDSP_NO_XSIMD
    return [] (const double* data, int numSamples)
    {
        auto squareSum = 0.0;
        for (int i = 0; i < numSamples; ++i)
            squareSum += data[i] * data[i];
        return std::sqrt (squareSum / (double) numSamples);
    }(src, numValues);
#else
    const auto squareSum = detail::reduce (src,
                                           numValues,
                                           0.0,
                                           [] (auto prev, auto next)
                                           { return prev + next * next; });
    return std::sqrt (squareSum / (double) numValues);
#endif
}

int countNaNs (const float* src, int numValues) noexcept
{
    return [] (const float* data, int numSamples)
    {
        int nanCount = 0;
        for (int i = 0; i < numSamples; ++i)
            nanCount += (int) std::isnan (data[i]);
        return nanCount;
    }(src, numValues);
}

int countNaNs (const double* src, int numValues) noexcept
{
    return [] (const double* data, int numSamples)
    {
        int nanCount = 0;
        for (int i = 0; i < numSamples; ++i)
            nanCount += (int) std::isnan (data[i]);
        return nanCount;
    }(src, numValues);
}

int countInfs (const float* src, int numValues) noexcept
{
    return [] (const float* data, int numSamples)
    {
        int nanCount = 0;
        for (int i = 0; i < numSamples; ++i)
            nanCount += (int) std::isinf (data[i]);
        return nanCount;
    }(src, numValues);
}

int countInfs (const double* src, int numValues) noexcept
{
    return [] (const double* data, int numSamples)
    {
        int nanCount = 0;
        for (int i = 0; i < numSamples; ++i)
            nanCount += (int) std::isinf (data[i]);
        return nanCount;
    }(src, numValues);
}

void rotate (float* data, int numToRotate, int totalNumValues, float* scratchData) noexcept
{
    juce::FloatVectorOperations::copy (scratchData, data, numToRotate);
    juce::FloatVectorOperations::copy (data, data + numToRotate, totalNumValues - numToRotate);
    juce::FloatVectorOperations::copy (data + totalNumValues - numToRotate, scratchData, numToRotate);
}

void rotate (double* data, int numToRotate, int totalNumValues, double* scratchData) noexcept
{
    juce::FloatVectorOperations::copy (scratchData, data, numToRotate);
    juce::FloatVectorOperations::copy (data, data + numToRotate, totalNumValues - numToRotate);
    juce::FloatVectorOperations::copy (data + totalNumValues - numToRotate, scratchData, numToRotate);
}
} // namespace chowdsp::FloatVectorOperations
