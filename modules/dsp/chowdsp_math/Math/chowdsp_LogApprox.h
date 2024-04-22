JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wstrict-aliasing")

namespace chowdsp
{
/**
 * Approximation functions for logarithmic functions.
 *
 * Note that these implementations require IEEE Standard 754
 * floating-point representations.
 */
namespace LogApprox
{
    namespace detail
    {
        using namespace Polynomials;
        using namespace SampleTypeHelpers;

        /** approximation for log_2(x), optimized on the range [1, 2] */
        template <typename T, int order>
        constexpr T log2_approx (T x)
        {
            // The first-order approximation just matches the endpoints (i.e. {0, 1} and {1, 2})
            // The second-order approximation matches the endpoints and the midpoint
            // The third-order approximation matches the endpoints, and the slopes at the endpoints

            static_assert (order >= 1 && order <= 3);

            if constexpr (order == 3)
            {
                constexpr auto alpha = (T) 0.1640425613334452;
                constexpr auto beta = (T) -1.098865286222744;
                constexpr auto gamma = (T) 3.148297929334117;
                constexpr auto zeta = (T) -2.213475204444817;
                return Polynomials::estrin<3> (Polynomial<T, 3> { { alpha, beta, gamma, zeta } }, x);
            }
            else if constexpr (order == 2)
            {
                constexpr auto beta = (T) -0.33985;
                constexpr auto gamma = (T) 2.01955;
                constexpr auto zeta = (T) -1.6797;
                return Polynomials::estrin<2> (Polynomial<T, 2> { { beta, gamma, zeta } }, x);
            }
            else if constexpr (order == 1)
            {
                return x - (T) 1;
            }
        }

#if ! CHOWDSP_NO_XSIMD
        /** approximation for log_2(x), optimized on the range [1, 2] */
        template <typename T, int order>
        inline xsimd::batch<T> log2_approx (const xsimd::batch<T>& x)
        {
            static_assert (order >= 1 && order <= 3);

            if constexpr (order == 3)
            {
                constexpr auto alpha = (T) 0.1640425613334452;
                constexpr auto beta = (T) -1.098865286222744;
                constexpr auto gamma = (T) 3.148297929334117;
                constexpr auto zeta = (T) -2.213475204444817;
                return Polynomials::estrin<3> (Polynomial<T, 3> { { alpha, beta, gamma, zeta } }, x);
            }
            else if constexpr (order == 2)
            {
                constexpr auto beta = (T) -0.33985;
                constexpr auto gamma = (T) 2.01955;
                constexpr auto zeta = (T) -1.6797;
                return Polynomials::estrin<2> (Polynomial<T, 2> { { beta, gamma, zeta } }, x);
            }
            else if constexpr (order == 1)
            {
                return x - (T) 1;
            }
        }
#endif

        using Euler = Ratio<std::intmax_t (1.0e18 * juce::MathConstants<double>::euler), std::intmax_t (1.0e18)>;
    } // namespace detail

    /** approximation for log(x) (32-bit) */
    template <typename Base, int order = 3>
    inline float logarithm (float x)
    {
        const auto vi = reinterpret_cast<int32_t&> (x); // NOSONAR
        const auto ex = vi & 0x7f800000;
        const auto e = (ex >> 23) - 127;
        const auto vfi = (vi - ex) | 0x3f800000;
        const auto vf = reinterpret_cast<const float&> (vfi); // NOSONAR

        static constexpr auto log2_base_r = 1.0f / gcem::log2 (Base::template value<float>);
        return log2_base_r * ((float) e + detail::log2_approx<float, order> (vf));
    }

    /** approximation for log(x) (64-bit) */
    template <typename Base, int order = 3>
    inline double logarithm (double x)
    {
        const auto vi = reinterpret_cast<int64_t&> (x); // NOSONAR
        const auto ex = vi & 0x7ff0000000000000;
        const auto e = (ex >> 52) - 1023;
        const auto vfi = (vi - ex) | 0x3ff0000000000000;
        const auto vf = reinterpret_cast<const double&> (vfi); // NOSONAR

        static constexpr auto log2_base_r = 1.0 / gcem::log2 (Base::template value<double>);
        return log2_base_r * ((double) e + detail::log2_approx<double, order> (vf));
    }

#if ! CHOWDSP_NO_XSIMD
    /** approximation for log(x) (SIMD 32-bit) */
    template <typename Base, int order = 3>
    inline xsimd::batch<float> logarithm (xsimd::batch<float> x)
    {
        const auto vi = reinterpret_cast<xsimd::batch<int32_t>&> (x); // NOSONAR
        const auto ex = vi & 0x7f800000;
        const auto e = (ex >> 23) - 127;
        const auto vfi = (vi - ex) | 0x3f800000;
        const auto vf = reinterpret_cast<const xsimd::batch<float>&> (vfi); // NOSONAR

        static constexpr auto log2_base_r = 1.0f / gcem::log2 (Base::template value<float>);
        return log2_base_r * (xsimd::to_float (e) + detail::log2_approx<float, order> (vf));
    }

    /** approximation for log(x) (SIMD 64-bit) */
    template <typename Base, int order = 3>
    inline xsimd::batch<double> logarithm (xsimd::batch<double> x)
    {
        const auto vi = reinterpret_cast<xsimd::batch<int64_t>&> (x); // NOSONAR
        const auto ex = vi & 0x7ff0000000000000;
        const auto e = (ex >> 52) - 1023;
        const auto vfi = (vi - ex) | 0x3ff0000000000000;
        const auto vf = reinterpret_cast<const xsimd::batch<double>&> (vfi); // NOSONAR

        static constexpr auto log2_base_r = 1.0 / gcem::log2 (Base::template value<double>);
        return log2_base_r * (xsimd::to_float (e) + detail::log2_approx<double, order> (vf));
    }
#endif

    template <typename T, int order = 3>
    inline T log (T x)
    {
        return logarithm<detail::Euler, order> (x);
    }

    template <typename T, int order = 3>
    inline T log10 (T x)
    {
        return logarithm<Ratio<10, 1>, order> (x);
    }

    template <typename T, int order = 3>
    inline T log2 (T x)
    {
        return logarithm<Ratio<2, 1>, order> (x);
    }
} // namespace LogApprox
} // namespace chowdsp

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
