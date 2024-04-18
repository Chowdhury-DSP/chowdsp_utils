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
     * Rounds a number up to the nearest multiple of the given multiplier.
     */
    template <typename T>
    constexpr T round_to_next_multiple (T value, T multiplier)
    {
        return ceiling_divide (value, multiplier) * multiplier;
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

    /** Returns 1 / sqrt (x) */
    template <typename T>
    inline std::enable_if_t<std::is_floating_point_v<T>, T> rsqrt (T x) noexcept
    {
        // We could do this with the Fast Inverse Square-Root,
        // but the compiler usually generates the equivalent
        // hardware instructions, particularly with -ffast-math.
        return (T) 1 / std::sqrt (x);
    }

#if ! CHOWDSP_NO_XSIMD
    /** Returns 1 / sqrt (x) */
    template <typename T>
    inline std::enable_if_t<SampleTypeHelpers::IsSIMDRegister<T>, T> rsqrt (T x) noexcept
    {
        return xsimd::rsqrt (x);
    }
#endif

    /** Algebraic sigmoid function of the form `y = x / sqrt(1 + x^2)` */
    template <typename T>
    inline T algebraicSigmoid (T x)
    {
        return x * rsqrt (x * x + (T) 1);
    }

    /** Returns true if the given number is a power of two. */
    constexpr bool isPowerOfTwo (int n)
    {
        return (n & (n - 1)) == 0;
    }
} // namespace Math
} // namespace chowdsp
