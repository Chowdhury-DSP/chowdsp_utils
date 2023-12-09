#pragma once

#include <cstdint>

namespace chowdsp
{
/** Helpers for generating random numbers. */
namespace RandomUtils
{
    template <typename T>
    struct BasisType;

    template <>
    struct BasisType<float>
    {
        using type = uint32_t;
    };

    template <>
    struct BasisType<double>
    {
        using type = uint64_t;
    };

#if ! CHOWDSP_NO_XSIMD
    template <>
    struct BasisType<xsimd::batch<float>>
    {
        using type = xsimd::batch<uint32_t>;
    };

    template <>
    struct BasisType<xsimd::batch<double>>
    {
        using type = xsimd::batch<uint64_t>;
    };
#endif

    template <typename T>
    using BasisTypeT = typename BasisType<T>::type;

    constexpr float rng_0_1 (uint32_t& seed)
    {
        seed = seed * 196314165UL + 907633515UL;
        return static_cast<float> (seed >> 8) / 16777216.0f;
    }

    constexpr double rng_0_1 (uint64_t& seed)
    {
        seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
        return static_cast<double> (seed >> 11) / 9007199254740992.0;
    }

#if ! CHOWDSP_NO_XSIMD
    inline xsimd::batch<float> rng_0_1 (xsimd::batch<uint32_t>& seed)
    {
        seed = seed * 196314165UL + 907633515UL;
        return xsimd::batch_cast<float> (seed >> 8) / 16777216.0f;
    }

    inline xsimd::batch<double> rng_0_1 (xsimd::batch<uint64_t>& seed)
    {
        seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
        return xsimd::batch_cast<double> (seed >> 11) / 9007199254740992.0;
    }
#endif

    constexpr float rng_m1_1 (uint32_t& seed)
    {
        seed = seed * 196314165UL + 907633515UL;
        auto temp = static_cast<int32_t> (seed >> 7) - 16777216L;
        return static_cast<float> (temp) / 16777216.0f;
    }

    constexpr double rng_m1_1 (uint64_t& seed)
    {
        seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
        auto temp = static_cast<int64_t> (seed >> 10) - 9007199254740992LL;
        return static_cast<double> (temp) / 9007199254740992.0;
    }

#if ! CHOWDSP_NO_XSIMD
    inline xsimd::batch<float> rng_m1_1 (xsimd::batch<uint32_t>& seed)
    {
        seed = seed * 196314165UL + 907633515UL;
        auto temp = xsimd::batch_cast<int32_t> (seed >> 7) - 16777216L;
        return xsimd::batch_cast<float> (temp) / 16777216.0f;
    }

    inline xsimd::batch<double> rng_m1_1 (xsimd::batch<uint64_t>& seed)
    {
        seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
        auto temp = xsimd::batch_cast<int64_t> (seed >> 10) - 9007199254740992LL;
        return xsimd::batch_cast<double> (temp) / 9007199254740992.0;
    }
#endif
} // namespace RandomUtils

/**
 * Wrapper class for generating random floating-point numbers.
 *
 * If you need numbers generated on a symmetric two-sided range,
 * (e.g. [-1, 1]), you'll get higher quality random numbers using
 * the two-sided distribution (see template arguments).
 *
 * Reference: https://audiodev.blog/random-numbers/
 */
template <typename T, bool twoSided = false>
class RandomFloat
{
public:
    using Basis = RandomUtils::BasisTypeT<T>;

    explicit RandomFloat (Basis s)
        : seed (s)
    {
    }

    T operator()()
    {
        if constexpr (twoSided)
            return RandomUtils::rng_m1_1 (seed);
        else
            return RandomUtils::rng_0_1 (seed);
    }

    template <bool TS = twoSided>
    std::enable_if_t<TS, T> operator() (T min, T max)
    {
        return min + (max - min) * RandomUtils::rng_0_1 (seed);
    }

private:
    Basis seed {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RandomFloat)
};
} // namespace chowdsp
