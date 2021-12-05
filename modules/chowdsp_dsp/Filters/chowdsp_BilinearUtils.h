#pragma once

#include <cmath>

namespace chowdsp
{
/**
 * Utility methods to digitize IIR filters with the bilinear transform.
 * The parameter K an be used to warp the bilinear transform or use K = 2 / T
 * for no warping.
 */
namespace Bilinear
{
    /** Dummy generic bilinear transform (@TODO: actually implement this) */
    template <typename T, size_t N>
    struct BilinearTransform
    {
        static inline void call (T (&b)[N], T (&a)[N], const T (&bs)[N], const T (&as)[N], T K);
    };

    /** Bilinear transform for a first-order filter */
    template <typename T>
    struct BilinearTransform<T, 2>
    {
        static inline void call (T (&b)[2], T (&a)[2], const T (&bs)[2], const T (&as)[2], T K)
        {
            const auto a0 = as[0] * K + as[1];
            b[0] = (bs[0] * K + bs[1]) / a0;
            b[1] = (-bs[0] * K + bs[1]) / a0;
            a[0] = 1.0f;
            a[1] = (-as[0] * K + as[1]) / a0;
        }
    };

    /** Bilinear transform for a second-order filter */
    template <typename T>
    struct BilinearTransform<T, 3>
    {
        static inline void call (T (&b)[3], T (&a)[3], const T (&bs)[3], const T (&as)[3], T K)
        {
            const auto KSq = K * K;
            const auto a0 = as[0] * KSq + as[1] * K + as[2];

            a[0] = 1.0f;
            a[1] = 2.0f * (as[2] - as[0] * KSq) / a0;
            a[2] = (as[0] * KSq - as[1] * K + as[2]) / a0;
            b[0] = (bs[0] * KSq + bs[1] * K + bs[2]) / a0;
            b[1] = 2.0f * (bs[2] - bs[0] * KSq) / a0;
            b[2] = (bs[0] * KSq - bs[1] * K + bs[2]) / a0;
        }
    };

    /** Computes the warping factor "K" so that the frequency fc is matched at sampel rate fs */
    template <typename T>
    inline T computeKValue (T fc, T fs)
    {
        const auto wc = juce::MathConstants<T>::twoPi * fc;
        return wc / std::tanh (wc / ((T) 2 * fs));
    }

    /** Calculates a pole frequency from a set of filter coefficients */
    template <typename T>
    inline T calcPoleFreq (T a, T b, T c)
    {
        auto radicand = b * b - 4 * a * c;
        if (radicand >= (T) 0)
            return (T) 0;

        return std::sqrt (-radicand) / (2 * a);
    }

} // namespace Bilinear

} // namespace chowdsp
