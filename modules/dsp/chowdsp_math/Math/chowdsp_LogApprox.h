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
        template <typename T>
        constexpr T log2_approx (T x)
        {
            constexpr auto alpha = (T) 0.1640425613334452;
            constexpr auto beta = (T) -1.098865286222744;
            constexpr auto gamma = (T) 3.148297929334117;
            constexpr auto zeta = (T) -2.213475204444817;

            return estrin<3> ({ alpha, beta, gamma, zeta }, x);
        }

#if ! CHOWDSP_NO_XSIMD
        /** approximation for log_2(x), optimized on the range [1, 2] */
        template <typename T>
        inline xsimd::batch<T> log2_approx (const xsimd::batch<T>& x)
        {
            static constexpr auto alpha = (NumericType<T>) 0.1640425613334452;
            static constexpr auto beta = (NumericType<T>) -1.098865286222744;
            static constexpr auto gamma = (NumericType<T>) 3.148297929334117;
            static constexpr auto zeta = (NumericType<T>) -2.213475204444817;

            return estrin<3> ({ alpha, beta, gamma, zeta }, x);
        }
#endif

        using Euler = Ratio<std::intmax_t (1.0e18 * juce::MathConstants<double>::euler), std::intmax_t (1.0e18)>;
    } // namespace detail

    /** approximation for log(x) (32-bit) */
    template <typename Base = detail::Euler>
    inline float log (float x)
    {
        const auto vi = reinterpret_cast<int32_t&> (x); // NOSONAR
        const auto ex = vi & 0x7f800000;
        const auto e = (ex >> 23) - 127;
        const auto vfi = (vi - ex) | 0x3f800000;
        const auto vf = reinterpret_cast<const float&> (vfi); // NOSONAR

        static constexpr auto log2_base_r = 1.0f / gcem::log2 (Base::template value<float>);
        return log2_base_r * ((float) e + detail::log2_approx<float> (vf));
    }

    /** approximation for log(x) (64-bit) */
    template <typename Base = detail::Euler>
    inline double log (double x)
    {
        const auto vi = reinterpret_cast<int64_t&> (x); // NOSONAR
        const auto ex = vi & 0x7ff0000000000000;
        const auto e = (ex >> 52) - 1023;
        const auto vfi = (vi - ex) | 0x3ff0000000000000;
        const auto vf = reinterpret_cast<const double&> (vfi); // NOSONAR

        static constexpr auto log2_base_r = 1.0 / gcem::log2 (Base::template value<double>);
        return log2_base_r * ((double) e + detail::log2_approx<double> (vf));
    }

#if ! CHOWDSP_NO_XSIMD
    /** approximation for log(x) (SIMD 32-bit) */
    template <typename Base = detail::Euler>
    inline xsimd::batch<float> log (xsimd::batch<float> x)
    {
        const auto vi = reinterpret_cast<xsimd::batch<int32_t>&> (x); // NOSONAR
        const auto ex = vi & 0x7f800000;
        const auto e = (ex >> 23) - 127;
        const auto vfi = (vi - ex) | 0x3f800000;
        const auto vf = reinterpret_cast<const xsimd::batch<float>&> (vfi); // NOSONAR

        static constexpr auto log2_base_r = 1.0f / gcem::log2 (Base::template value<float>);
        return log2_base_r * ((xsimd::batch<float>) e + detail::log2_approx<float> (vf));
    }

    /** approximation for log(x) (SIMD 64-bit) */
    template <typename Base = detail::Euler>
    inline xsimd::batch<double> log (xsimd::batch<double> x)
    {
        const auto vi = reinterpret_cast<xsimd::batch<int64_t>&> (x); // NOSONAR
        const auto ex = vi & 0x7ff0000000000000;
        const auto e = (ex >> 52) - 1023;
        const auto vfi = (vi - ex) | 0x3ff0000000000000;
        const auto vf = reinterpret_cast<const xsimd::batch<double>&> (vfi); // NOSONAR

        static constexpr auto log2_base_r = 1.0 / gcem::log2 (Base::template value<double>);
        return log2_base_r * ((xsimd::batch<double>) e + detail::log2_approx<double> (vf));
    }
#endif

    template <typename T>
    inline T log10 (T x)
    {
        return log<Ratio<10, 1>> (x);
    }

    template <typename T>
    inline T log2 (T x)
    {
        return log<Ratio<2, 1>> (x);
    }
} // namespace LogApprox
} // namespace chowdsp
