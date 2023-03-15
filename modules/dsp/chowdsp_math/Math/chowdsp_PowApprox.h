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
        template <typename T>
        constexpr T pow2_approx (T x)
        {
            constexpr auto alpha = (T) 0.07944154167983575;
            constexpr auto beta = (T) 0.2274112777602189;
            constexpr auto gamma = (T) 0.6931471805599453;
            constexpr auto zeta = (T) 1.0;

            return estrin<3> ({ alpha, beta, gamma, zeta }, x);
        }

#if ! CHOWDSP_NO_XSIMD
        /** approximation for 2^x, optimized on the range [0, 1] */
        template <typename T>
        inline xsimd::batch<T> pow2_approx (const xsimd::batch<T>& x)
        {
            static constexpr auto alpha = (NumericType<T>) 0.07944154167983575;
            static constexpr auto beta = (NumericType<T>) 0.2274112777602189;
            static constexpr auto gamma = (NumericType<T>) 0.6931471805599453;
            static constexpr auto zeta = (NumericType<T>) 1.0;

            return estrin<3> ({ alpha, beta, gamma, zeta }, x);
        }
#endif

        using Euler = Ratio<std::intmax_t (1.0e18 * juce::MathConstants<double>::euler), std::intmax_t (1.0e18)>;
    } // namespace detail

    /** approximation for pow(Base, x) (32-bit) */
    template <typename Base>
    inline float pow (float x)
    {
        static constexpr auto log2_base = gcem::log2 (Base::template value<float>);
        x = std::max (-126.0f, log2_base * x);

        const auto xi = (int32_t) x;
        const auto l = x < 0.0f ? xi - 1 : xi;
        const auto f = x - (float) l;
        const auto vi = (l + 127) << 23;

        return reinterpret_cast<const float&> (vi) * detail::pow2_approx<float> (f); // NOSONAR
    }

    /** approximation for pow(Base, x) (64-bit) */
    template <typename Base>
    inline double pow (double x)
    {
        static constexpr auto log2_base = gcem::log2 (Base::template value<double>);
        x = std::max (-1022.0, log2_base * x);

        const auto xi = (int64_t) x;
        const auto l = x < 0.0 ? xi - 1 : xi;
        const auto d = x - (double) l;
        const auto vi = (l + 1023) << 52;

        return reinterpret_cast<const double&> (vi) * detail::pow2_approx<double> (d); // NOSONAR
    }

#if ! CHOWDSP_NO_XSIMD
    /** approximation for pow(Base, x) (SIMD 32-bit) */
    template <typename Base>
    inline xsimd::batch<float> pow (xsimd::batch<float> x)
    {
        static constexpr auto log2_base = gcem::log2 (Base::template value<float>);
        x = xsimd::max (xsimd::broadcast (-126.0f), log2_base * x);

        const auto xi = xsimd::to_int (x);
        const auto l = xsimd::select (xsimd::batch_bool_cast<int32_t> (x < 0.0f), xi - 1, xi);
        const auto f = x - xsimd::to_float (l);
        const auto vi = (l + 127) << 23;

        return reinterpret_cast<const xsimd::batch<float>&> (vi) * detail::pow2_approx<float> (f);
    }

    /** approximation for pow(Base, x) (SIMD 64-bit) */
    template <typename Base>
    inline xsimd::batch<double> pow (xsimd::batch<double> x)
    {
        static constexpr auto log2_base = gcem::log2 (Base::template value<double>);
        x = xsimd::max (xsimd::broadcast (-1022.0), log2_base * x);

        const auto xi = xsimd::to_int (x);
        const auto l = xsimd::select (xsimd::batch_bool_cast<int64_t> (x < 0.0), xi - 1, xi);
        const auto d = x - xsimd::to_float (l);
        const auto vi = (l + 1023) << 52;

        return reinterpret_cast<const xsimd::batch<double>&> (vi) * detail::pow2_approx<double> (d);
    }
#endif

    template <typename T>
    inline T exp (T x)
    {
        return pow<detail::Euler> (x);
    }

    template <typename T>
    inline T pow2 (T x)
    {
        return pow<Ratio<2, 1>> (x);
    }

    template <typename T>
    inline T pow10 (T x)
    {
        return pow<Ratio<10, 1>> (x);
    }
} // namespace PowApprox
} // namespace chowdsp
