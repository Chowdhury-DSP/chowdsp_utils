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
                auto val_b = T {};
                auto val_a = T {};
                auto k_val = (T) 1;

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
    template <typename T, typename NumericType>
    inline T computeKValueAngular (T wc, NumericType fs)
    {
        CHOWDSP_USING_XSIMD_STD (tan);
        return wc / tan (wc / ((NumericType) 2 * fs));
    }

    /** Computes the warping factor "K" so that the frequency fc is matched at sample rate fs */
    template <typename T, typename NumericType>
    inline T computeKValue (T fc, NumericType fs)
    {
        const auto wc = juce::MathConstants<NumericType>::twoPi * fc;
        return computeKValueAngular (wc, fs);
    }

    /** Clamps a Q-value into the range where Vicanek's method is numerically robust */
    template <typename T>
    [[maybe_unused]] inline T clampQVicanek (T qVal)
    {
        CHOWDSP_USING_XSIMD_STD (max);
        CHOWDSP_USING_XSIMD_STD (min);

        // Vicanek method has numerical problems for Q-values outside of range [0.55, 4.95]
        // @TODO: can we make this range looser when using double-precision?
        jassert (SIMDUtils::all (qVal >= (T) 0.55));
        jassert (SIMDUtils::all (qVal < (T) 4.95));
        return min ((T) 4.95, max ((T) 0.55, qVal));
    }

    /** Computes denominator coefficients using Vicanek's method. Using w0 = 2 * pi * fc / fs */
    template <typename T>
    [[maybe_unused]] inline void computeVicanekDenominator (T w0, T qVal, T (&a)[3])
    {
        CHOWDSP_USING_XSIMD_STD (sqrt);
        CHOWDSP_USING_XSIMD_STD (exp);
        CHOWDSP_USING_XSIMD_STD (cos);
        CHOWDSP_USING_XSIMD_STD (cosh);
        using Power::ipow;

        const auto inv2Q = (T) 0.5 / qVal;

        const auto expmqw = exp (-inv2Q * w0);
        const auto cos_arg = sqrt ((T) 1 - ipow<2> (inv2Q)) * w0;
        a[0] = (T) 1;
        a[1] = -(T) 2 * expmqw * SIMDUtils::select (inv2Q <= (T) 1, cos (cos_arg), cosh (-cos_arg));
        a[2] = ipow<2> (expmqw);
    }

    /** Computes the phi values used by Vicanek's method. Using w0 = 2 * pi * fc / fs */
    template <typename T>
    [[maybe_unused]] inline auto computeVicanekPhiVals (T w0)
    {
        CHOWDSP_USING_XSIMD_STD (sin);
        using Power::ipow;

        const auto sinpd2 = sin ((T) 0.5 * w0);
        const auto p1 = ipow<2> (sinpd2);
        const auto p0 = (T) 1 - p1;
        const auto p2 = (T) 4 * p0 * p1;

        return std::make_tuple (p0, p1, p2);
    }

    /** Computes the "A" values used by Vicanek's method. */
    template <typename T>
    [[maybe_unused]] inline auto computeVicanekAVals (const T (&a)[3])
    {
        using Power::ipow;
        const auto A0 = ipow<2> ((T) 1 + a[1] + a[2]);
        const auto A1 = ipow<2> ((T) 1 - a[1] + a[2]);
        const auto A2 = (T) -4 * a[2];

        return std::make_tuple (A0, A1, A2);
    }

    /** Computes pole info using Vicanek's method */
    template <typename T, typename NumericType>
    [[maybe_unused]] inline auto computeVicanekPolesAngular (T wc, T qVal, NumericType fs, T (&a)[3])
    {
        const auto w0 = wc / fs;
        computeVicanekDenominator (w0, qVal, a);

        const auto [p0, p1, p2] = computeVicanekPhiVals (w0);
        const auto [A0, A1, A2] = computeVicanekAVals (a);

        return std::make_tuple (p0, p1, p2, A0, A1, A2);
    }

    /** Calculates a pole frequency from a set of filter coefficients */
    template <typename T>
    [[maybe_unused]] inline T calcPoleFreq (T a, T b, T c)
    {
        auto radicand = b * b - 4 * a * c;
        if (radicand >= (T) 0)
            return (T) 0;

        return std::sqrt (-radicand) / (2 * a);
    }
} // namespace ConformalMaps
} // namespace chowdsp
