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
    void unaryOpFallback (T* dest, const T* src, int numValues, Op&& op)
    {
        for (int i = 0; i < numValues; ++i)
            dest[i] = op (src[i]);
    }

    template <typename T, typename ScalarOp, typename VecOp, typename LoadOpType, typename StoreOpType>
    void unaryOp (T* dest, const T* src, int numValues, ScalarOp&& scalarOp, VecOp&& vecOp, LoadOpType&& loadOp, StoreOpType&& storeOp)
    {
        constexpr auto vecSize = (int) juce::dsp::SIMDRegister<T>::size();
        auto numVecOps = numValues / vecSize;

        // Fallback: not enough operations to justify vectorizing!
        if (numVecOps < 2)
        {
            unaryOpFallback (dest, src, numValues, scalarOp);
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
            unaryOpFallback (dest, src, leftoverValues, scalarOp);
    }

    template <typename T, typename ScalarOp, typename VecOp>
    void unaryOp (T* dest, const T* src, int numValues, ScalarOp&& scalarOp, VecOp&& vecOp)
    {
        auto loadA = [] (const auto* ptr)
        { return juce::dsp::SIMDRegister<T>::fromRawArray (ptr); };

        auto loadU = [] (const auto* ptr)
        { return SIMDUtils::loadUnaligned (ptr); };

        auto storeA = [] (auto* ptr, const auto& reg)
        { return reg.copyToRawArray (ptr); };

        auto storeU = [] (auto* ptr, const auto& reg)
        { return SIMDUtils::storeUnaligned (ptr, reg); };

        if (isAligned (dest))
        {
            if (isAligned (src))
                unaryOp (dest, src, numValues, scalarOp, vecOp, loadA, storeA);
            else
                unaryOp (dest, src, numValues, scalarOp, vecOp, loadU, storeA);
        }
        else
        {
            if (isAligned (src))
                unaryOp (dest, src, numValues, scalarOp, vecOp, loadA, storeU);
            else
                unaryOp (dest, src, numValues, scalarOp, vecOp, loadU, storeU);
        }
    }

    template <typename T, typename Op>
    void unaryOp (T* dest, const T* src, int numValues, Op&& op)
    {
        unaryOp (dest, src, numValues, op, op);
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
        return reduce (src, numValues, init, scalarOp, vecOp, [] (auto val)
                       { return val.sum(); });
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
        auto vecLoop = [&] (auto&& loadOp1, auto&& loadOp2)
        {
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
        auto loadA = [] (const T* val)
        { return juce::dsp::SIMDRegister<T>::fromRawArray (val); };
        auto loadU = [] (const T* val)
        { return loadUnaligned (val); };

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
        return reduce (src1, src2, numValues, init, scalarOp, vecOp, [] (auto val)
                       { return val.sum(); });
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
        [] (auto prev, auto next)
        { return prev + next; });
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
#else
    using Vec = juce::dsp::SIMDRegister<float>;
    return detail::reduce (
        src,
        numValues,
        0.0f,
        [] (auto a, auto b)
        { return juce::jmax (std::abs (a), std::abs (b)); },
        [] (auto a, auto b)
        { return Vec::max (Vec::abs (a), Vec::abs (b)); },
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
#else
    using Vec = juce::dsp::SIMDRegister<double>;
    return detail::reduce (
        src,
        numValues,
        0.0,
        [] (auto a, auto b)
        { return juce::jmax (a, std::abs (b)); },
        [] (auto a, auto b)
        { return Vec::max (a, Vec::abs (b)); },
        [] (auto x)
        { return SIMDUtils::hMaxSIMD (x); });
#endif
}

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
                dest, src, numValues, [exponent] (auto x)
                { return std::pow (x, (T) exponent); },
                [exponent] (auto x)
                { return SIMDUtils::powSIMD (x, juce::dsp::SIMDRegister<T> ((T) exponent)); });
            break;
    }
}

void integerPower (float* dest, const float* src, int exponent, int numValues) noexcept
{
    integerPowerT (dest, src, exponent, numValues);
}

void integerPower (double* dest, const double* src, int exponent, int numValues) noexcept
{
    integerPowerT (dest, src, exponent, numValues);
}
} // namespace chowdsp::FloatVectorOperations
