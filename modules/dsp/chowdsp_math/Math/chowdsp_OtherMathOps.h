#pragma once

namespace chowdsp
{
/** A handful of extra maths functions */
namespace Math
{
    /**
     * log2 for integer values.
     *
     * For numbers that are not a power of two, this method will round up.
     */
    template <int n>
    constexpr int log2()
    {
        static_assert (n > 0, "Log2 is undefined for numbers less than or equal to zero!");
        if constexpr (n <= 1)
            return 0;
        else if constexpr (n == 2)
            return 1; // gcem::log2 behaves poorly in this case
        else
            return 2 + (int) gcem::log2 ((n - 1) / 2);
    }

    /**
     * log2 for integer values.
     *
     * For numbers that are not a power of two, this method will round up.
     */
    template <typename IntType>
    inline int log2 (IntType n)
    {
        jassert (n > 0); // Log2 is undefined for numbers less than or equal to zero!"
        return n <= 1 ? 0 : (2 + (int) std::log2 (((double) n - 1.0) / 2.0));
    }

    /**
     * Divides two numbers and rounds up if there is a remainder.
     *
     * This is often useful for figuring out haw many SIMD registers are needed
     * to contain a given number of scalar values.
     */
    template <typename T>
    constexpr T ceiling_divide (T num, T den)
    {
        return (num + den - 1) / den;
    }

    /**
     * Returns 1 if the input is positive, -1 if the input is negative,
     * and 0 if the input is zero.
     */
    template <typename T>
    constexpr T sign (T val)
    {
        return T ((T (0) < val) - (val < T (0)));
    }

#if ! CHOWDSP_NO_XSIMD
    /**
     * Returns 1 if the input is positive, -1 if the input is negative,
     * and 0 if the input is zero.
     */
    template <typename T>
    inline xsimd::batch<T> sign (xsimd::batch<T> val)
    {
        using v_type = xsimd::batch<T>;
        const auto positive = xsimd::select (val > v_type ((T) 0), v_type ((T) 1), v_type ((T) 0));
        const auto negative = xsimd::select (val < v_type ((T) 0), v_type ((T) 1), v_type ((T) 0));
        return positive - negative;
    }
#endif

    /** The famous "Fast Inverse Square-Root" method (https://en.wikipedia.org/wiki/Fast_inverse_square_root) */
    template <typename T, typename NumericType = SampleTypeHelpers::NumericType<T>>
    inline std::enable_if_t<std::is_same_v<NumericType, float>, T> rsqrt (T x) noexcept
    {
#if CHOWDSP_NO_XSIMD
        using IntType = int32_t;
#else
        using IntType = std::conditional_t<SampleTypeHelpers::IsSIMDRegister<T>, xsimd::batch<int32_t>, int32_t>;
#endif

        const auto x_half = x * 0.5f;
        auto i = reinterpret_cast<IntType&> (x);
        i = 0x5f3759df - (i >> 1);

        auto y = reinterpret_cast<T&> (i);
        y = y * (1.5f - (x_half * y * y)); // 1st iteration
        // y  = y * (1.5f - (x_half * y * y)); // 2nd iteration, this can be removed

        return y;
    }

    /** The famous "Fast Inverse Square-Root" method (re-written for double-precision) */
    template <typename T, typename NumericType = SampleTypeHelpers::NumericType<T>>
    inline std::enable_if_t<std::is_same_v<NumericType, double>, T> rsqrt (T x) noexcept
    {
#if CHOWDSP_NO_XSIMD
        using IntType = int64_t;
#else
        using IntType = std::conditional_t<SampleTypeHelpers::IsSIMDRegister<T>, xsimd::batch<int64_t>, int64_t>;
#endif

        const auto x_half = x * 0.5;
        auto i = reinterpret_cast<IntType&> (x);
        i = 0x5fe6eb50c7b537a9 - (i >> 1); // The magic number is for doubles is from https://cs.uwaterloo.ca/~m32rober/rsqrt.pdf

        auto y = reinterpret_cast<T&> (i);
        y = y * (1.5 - (x_half * y * y)); // 1st iteration
        // y  = y * (1.5 - (x_half * y * y)); // 2nd iteration, this can be removed

        return y;
    }

    /** Algebraic sigmoid function of the form `y = x / sqrt(1 + x^2)` */
    template <typename T>
    inline T algebraicSigmoid (T x)
    {
        return x * rsqrt (x * x + (T) 1);
    }
} // namespace Math
} // namespace chowdsp
