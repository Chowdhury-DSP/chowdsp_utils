#pragma once

#include <cmath>

namespace chowdsp
{
/**
 * Utility methods to digitize IIR filters with a conformal map.
 *
 * The parameter K an be used to frequency warp the transform or use K = 2 / T
 * for no warping.
 */
namespace ConformalMaps
{
    /** Transforms for a M order filter */
    template <typename T, int M>
    struct Transform
    {
        static constexpr int N = M + 1;

        static inline void bilinear (T (&b)[(size_t) N], T (&a)[(size_t) N], const T (&bs)[(size_t) N], const T (&as)[(size_t) N], T K)
        {
            using Combinatorics::combination, Power::ipow;

            for (int j = 0; j < N; ++j)
            {
                T val_b {};
                T val_a {};
                T k_val = (T) 1;

                for (int i = 0; i < N; ++i)
                {
                    int n1_pow = 1;
                    for (int k = 0; k < i + 1; ++k)
                    {
                        const auto comb_i_k = combination (i, k);
                        const auto k_pow = k_val * n1_pow;

                        for (int l = 0; l < N - i; ++l)
                        {
                            if (k + l != j)
                                continue;

                            const auto coeff_mult = comb_i_k * combination (M - i, l) * k_pow;
                            val_b += coeff_mult * bs[M - i];
                            val_a += coeff_mult * as[M - i];
                        }

                        n1_pow *= -1;
                    }

                    k_val *= K;
                }

                b[j] = val_b;
                a[j] = val_a;
            }

            // normalize coefficients
            for (int j = N - 1; j >= 0; --j)
            {
                b[j] /= a[0];
                a[j] /= a[0];
            }
        }
    };

    /** Parameters for the Mobius transform. */
    template <typename T>
    struct MobiusParams
    {
        T a, b, c, d;
    };

    /** Transforms for a first-order filter */
    template <typename T>
    struct Transform<T, 1>
    {
        /** First-order Mobius transform */
        static inline void mobius (T (&b)[2], T (&a)[2], const T (&bs)[2], const T (&as)[2], const MobiusParams<T>& mb)
        {
            const auto a0_inv = (T) 1 / (as[0] * mb.a + as[1] * mb.c);
            b[0] = (bs[0] * mb.a + bs[1] * mb.c) * a0_inv;
            b[1] = (bs[0] * mb.b + bs[1] * mb.d) * a0_inv;
            a[0] = (T) 1;
            a[1] = (as[0] * mb.b + as[1] * mb.d) * a0_inv;
        }

        /** First-order bilinear transform */
        static inline void bilinear (T (&b)[2], T (&a)[2], const T (&bs)[2], const T (&as)[2], T K)
        {
            mobius (b, a, bs, as, { K, -K, (T) 1, (T) 1 });
        }

        /** First-order alpha transform */
        static inline void alpha (T (&b)[2], T (&a)[2], const T (&bs)[2], const T (&as)[2], T K, T alpha, T fs)
        {
            const auto Kprime = K / ((T) 2 * fs);
            const auto alphaA = Kprime * ((T) 1 + alpha) * fs;
            mobius (b, a, bs, as, { alphaA, -alphaA, (T) 1, alpha });
        }
    };

    /** Transforms for a second-order filter */
    template <typename T>
    struct Transform<T, 2>
    {
        /** Second-order Mobius transform */
        static inline void mobius (T (&b)[3], T (&a)[3], const T (&bs)[3], const T (&as)[3], const MobiusParams<T>& mb)
        {
            const auto mbAsq = mb.a * mb.a;
            const auto mbAC = mb.a * mb.c;
            const auto mbCsq = mb.c * mb.c;
            const auto mb2AB = (T) 2 * mb.a * mb.b;
            const auto mbAD_BC = mb.a * mb.d + mb.b * mb.c;
            const auto mb2CD = (T) 2 * mb.c * mb.d;
            const auto mbBsq = mb.b * mb.b;
            const auto mbBD = mb.b * mb.d;
            const auto mbDsq = mb.d * mb.d;

            const auto a0_inv = (T) 1 / (as[0] * mbAsq + as[1] * mbAC + as[2] * mbCsq);
            b[0] = (bs[0] * mbAsq + bs[1] * mbAC + bs[2] * mbCsq) * a0_inv;
            b[1] = (bs[0] * mb2AB + bs[1] * mbAD_BC + bs[2] * mb2CD) * a0_inv;
            b[2] = (bs[0] * mbBsq + bs[1] * mbBD + bs[2] * mbDsq) * a0_inv;
            a[0] = (T) 1;
            a[1] = (as[0] * mb2AB + as[1] * mbAD_BC + as[2] * mb2CD) * a0_inv;
            a[2] = (as[0] * mbBsq + as[1] * mbBD + as[2] * mbDsq) * a0_inv;
        }

        /** Second-order bilinear transform */
        static inline void bilinear (T (&b)[3], T (&a)[3], const T (&bs)[3], const T (&as)[3], T K)
        {
            // the mobius transform adds a little computational overhead, so here's the optimized bilinear transform.
            const auto KSq = K * K;
            const auto a0_inv = (T) 1 / (as[0] * KSq + as[1] * K + as[2]);

            a[0] = (T) 1;
            a[1] = (T) 2 * (as[2] - as[0] * KSq) * a0_inv;
            a[2] = (as[0] * KSq - as[1] * K + as[2]) * a0_inv;
            b[0] = (bs[0] * KSq + bs[1] * K + bs[2]) * a0_inv;
            b[1] = (T) 2 * (bs[2] - bs[0] * KSq) * a0_inv;
            b[2] = (bs[0] * KSq - bs[1] * K + bs[2]) * a0_inv;
        }

        /** Second-order alpha transform */
        static inline void alpha (T (&b)[3], T (&a)[3], const T (&bs)[3], const T (&as)[3], T K, T alpha, T fs)
        {
            const auto Kprime = K / ((T) 2 * fs);
            const auto alphaA = Kprime * ((T) 1 + alpha) * fs;
            mobius (b, a, bs, as, { alphaA, -alphaA, (T) 1, alpha });
        }
    };

    /** Computes the warping factor "K" so that the angular frequency wc is matched at sample rate fs */
    template <typename T>
    inline T computeKValueAngular (T wc, SampleTypeHelpers::NumericType<T> fs)
    {
        using NumericType = SampleTypeHelpers::NumericType<T>;
        CHOWDSP_USING_XSIMD_STD (tan)

        return wc / tan (wc / ((NumericType) 2 * fs));
    }

    /** Computes the warping factor "K" so that the frequency fc is matched at sample rate fs */
    template <typename T>
    inline T computeKValue (T fc, SampleTypeHelpers::NumericType<T> fs)
    {
        using NumericType = SampleTypeHelpers::NumericType<T>;
        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;
        return computeKValueAngular (wc, fs);
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
} // namespace ConformalMaps
} // namespace chowdsp
