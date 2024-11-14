JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wstrict-aliasing")

namespace chowdsp
{
/**
 * Approximation functions for power functions.
 *
 * Note that these implementations require IEEE Standard 754
 * floating-point representations.
 */
namespace PowApprox
{
    namespace detail
    {
        using namespace Polynomials;
        using namespace SampleTypeHelpers;

        /** approximation for 2^x, optimized on the range [0, 1] */
        template <typename T, int order>
        constexpr T pow2_approx (T x)
        {
            // The first-order approximation just matches the endpoints (i.e. {0, 1} and {1, 2})
            // The second-order approximation matches the endpoints and the midpoint
            // The third-order approximation matches the endpoints, and the slopes at the endpoints

            static_assert (order >= 1 && order <= 3);

            if constexpr (order == 3)
            {
                constexpr auto alpha = (T) 0.07944154167983575;
                constexpr auto beta = (T) 0.2274112777602189;
                constexpr auto gamma = (T) 0.6931471805599453;
                constexpr auto zeta = (T) 1.0;
                return Polynomials::estrin<3> (Polynomial<T, 3> { { alpha, beta, gamma, zeta } }, x);
            }
            else if constexpr (order == 2)
            {
                constexpr auto beta = (T) 0.343146;
                constexpr auto gamma = (T) 0.656854;
                constexpr auto zeta = (T) 1.0;
                return Polynomials::estrin<2> (Polynomial<T, 2> { { beta, gamma, zeta } }, x);
            }
            else if constexpr (order == 1)
            {
                return x + (T) 1;
            }
        }

#if ! CHOWDSP_NO_XSIMD
        /** approximation for 2^x, optimized on the range [0, 1] */
        template <typename T, int order>
        inline xsimd::batch<T> pow2_approx (const xsimd::batch<T>& x)
        {
            static_assert (order >= 1 && order <= 3);

            if constexpr (order == 3)
            {
                constexpr auto alpha = (T) 0.07944154167983575;
                constexpr auto beta = (T) 0.2274112777602189;
                constexpr auto gamma = (T) 0.6931471805599453;
                constexpr auto zeta = (T) 1.0;
                return Polynomials::estrin<3> (Polynomial<T, 3> { { alpha, beta, gamma, zeta } }, x);
            }
            else if constexpr (order == 2)
            {
                constexpr auto beta = (T) 0.343146;
                constexpr auto gamma = (T) 0.656854;
                constexpr auto zeta = (T) 1.0;
                return Polynomials::estrin<2> (Polynomial<T, 2> { { beta, gamma, zeta } }, x);
            }
            else if constexpr (order == 1)
            {
                return x + (T) 1;
            }
        }
#endif

        using Euler = Ratio<std::intmax_t (1.0e18 * juce::MathConstants<double>::euler), std::intmax_t (1.0e18)>;
    } // namespace detail

    /** approximation for pow(Base, x) (32-bit) */
    template <typename Base, int order>
    inline float pow (float x)
    {
        static constexpr auto log2_base = gcem::log2 (Base::template value<float>);
        x = std::max (-126.0f, log2_base * x);

        const auto xi = (int32_t) x;
        const auto l = x < 0.0f ? xi - 1 : xi;
        const auto f = x - (float) l;
        const auto vi = (l + 127) << 23;

        return reinterpret_cast<const float&> (vi) * detail::pow2_approx<float, order> (f); // NOSONAR
    }

    /** approximation for pow(Base, x) (64-bit) */
    template <typename Base, int order>
    inline double pow (double x)
    {
        static constexpr auto log2_base = gcem::log2 (Base::template value<double>);
        x = std::max (-1022.0, log2_base * x);

        const auto xi = (int64_t) x;
        const auto l = x < 0.0 ? xi - 1 : xi;
        const auto d = x - (double) l;
        const auto vi = (l + 1023) << 52;

        return reinterpret_cast<const double&> (vi) * detail::pow2_approx<double, order> (d); // NOSONAR
    }

#if ! CHOWDSP_NO_XSIMD
    /** approximation for pow(Base, x) (SIMD 32-bit) */
    template <typename Base, int order>
    inline xsimd::batch<float> pow (xsimd::batch<float> x)
    {
        static constexpr auto log2_base = gcem::log2 (Base::template value<float>);
        x = xsimd::max (xsimd::broadcast (-126.0f), log2_base * x);

        const auto xi = xsimd::to_int (x);
        const auto l = xsimd::select (xsimd::batch_bool_cast<int32_t> (x < 0.0f), xi - 1, xi);
        const auto f = x - xsimd::to_float (l);
        const auto vi = (l + 127) << 23;

        return reinterpret_cast<const xsimd::batch<float>&> (vi) * detail::pow2_approx<float, order> (f); // NOSONAR
    }

    /** approximation for pow(Base, x) (SIMD 64-bit) */
    template <typename Base, int order>
    inline xsimd::batch<double> pow (xsimd::batch<double> x)
    {
        static constexpr auto log2_base = gcem::log2 (Base::template value<double>);
        x = xsimd::max (xsimd::broadcast (-1022.0), log2_base * x);

        const auto xi = xsimd::to_int (x);
        const auto l = xsimd::select (xsimd::batch_bool_cast<int64_t> (x < 0.0), xi - 1, xi);
        const auto d = x - xsimd::to_float (l);
        const auto vi = (l + 1023) << 52;

        return reinterpret_cast<const xsimd::batch<double>&> (vi) * detail::pow2_approx<double, order> (d); // NOSONAR
    }
#endif

    template <typename T, int order = 3>
    inline T exp (T x)
    {
        return pow<detail::Euler, order> (x);
    }

    template <typename T, int order = 3>
    inline T pow2 (T x)
    {
        return pow<Ratio<2, 1>, order> (x);
    }

    template <typename T, int order = 3>
    inline T pow10 (T x)
    {
        return pow<Ratio<10, 1>, order> (x);
    }
} // namespace PowApprox
} // namespace chowdsp
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
