#pragma once

namespace chowdsp
{
/**
 * Bucket-Bridage Device emulation, as derived by
 * Martin Holters and Julian Parker:
 * http://dafx2018.web.ua.pt/papers/DAFx2018_paper_12.pdf
 */
namespace BBD
{
    template <typename T>
    using SIMDScalar = xsimd::batch<T>;

    template <typename T>
    using SIMDComplex = xsimd::batch<std::complex<T>>;

    /**
 * Anti-aliasing/reconstruction filters used by JUNO-60 chorus.
 * Root/Pole analysis borrowed from the above paper.
 */
    namespace BBDFilterSpec
    {
        constexpr float inputFilterOriginalCutoff = 9900.0f;
        constexpr float outputFilterOriginalCutoff = 9500.0f;

        constexpr std::complex<float> iFiltRoot alignas (xsimd::default_arch::alignment())[] = {
            { -10329.2715f, -329.848f },
            { -10329.2715f, +329.848f },
            { 366.990557f, -1811.4318f },
            { 366.990557f, +1811.4318f }
        };

        constexpr std::complex<float> iFiltPole alignas (xsimd::default_arch::alignment())[] = {
            { -55482.0f, -25082.0f },
            { -55482.0f, +25082.0f },
            { -26292.0f, -59437.0f },
            { -26292.0f, +59437.0f }
        };

        constexpr std::complex<float> oFiltRoot alignas (xsimd::default_arch::alignment())[] = {
            { -11256.0f, -99566.0f },
            { -11256.0f, +99566.0f },
            { -13802.0f, -24606.0f },
            { -13802.0f, +24606.0f }
        };
        constexpr std::complex<float> oFiltPole alignas (xsimd::default_arch::alignment())[] = {
            { -51468.0f, -21437.0f },
            { -51468.0f, +21437.0f },
            { -26276.0f, -59699.0f },
            { -26276.0f, +59699.0f }
        };

        inline SIMDComplex<float> fast_complex_pow (SIMDScalar<float> angle, float b)
        {
            auto [sinAngle, cosAngle] = xsimd::sincos (angle * b);
            return { cosAngle, sinAngle };
        }
    } // namespace BBDFilterSpec

    /** Filter bank for BBD signal input. */
    class InputFilterBank
    {
        using T = float; // We need SIMD size 4 (for now)
        using Complex4 = SIMDComplex<T>;

    public:
        explicit InputFilterBank (T sampleTime) : Ts (sampleTime)
        {
            roots = xsimd::load_aligned (BBDFilterSpec::iFiltRoot);
            poles = xsimd::load_aligned (BBDFilterSpec::iFiltPole);
        }

        inline void set_freq (float freq)
        {
            const float freqFactor = freq / BBDFilterSpec::inputFilterOriginalCutoff;
            root_corr = roots * freqFactor;
            pole_corr = xsimd::exp (poles * (freqFactor * Ts));
            pole_corr_angle = xsimd::arg (pole_corr);

            gCoef = root_corr * Ts;
        }

        inline void set_time (float tn) noexcept
        {
            Gcalc = gCoef * SIMDUtils::pow (pole_corr, SIMDScalar<float> (tn));
        }

        inline void set_delta (float delta) noexcept
        {
            Aplus = BBDFilterSpec::fast_complex_pow (pole_corr_angle, delta);
        }

        inline void calcG() noexcept { Gcalc = Aplus * Gcalc; }

        inline void process (float u) noexcept
        {
            x = pole_corr * x + Complex4 { SIMDScalar<T> (u), SIMDScalar<T> (0) };
        }

        Complex4 x {};
        Complex4 Gcalc { SIMDScalar<T> (1), SIMDScalar<T> (0) };

    private:
        Complex4 roots {};
        Complex4 poles {};
        Complex4 root_corr {};
        Complex4 pole_corr {};
        xsimd::batch<T> pole_corr_angle {};

        Complex4 Aplus {};

        const T Ts;
        Complex4 gCoef {};
    };

    /** Filter bank for BBD signal output. */
    class OutputFilterBank
    {
        using T = float; // We need SIMD size 4 (for now)
        using Complex4 = SIMDComplex<T>;

    public:
        explicit OutputFilterBank (float sampleTime) : Ts (sampleTime)
        {
            gCoef = xsimd::load_aligned (BBDFilterSpec::oFiltRoot) / xsimd::load_aligned (BBDFilterSpec::oFiltPole);
            poles = xsimd::load_aligned (BBDFilterSpec::oFiltPole);
        }

        [[nodiscard]] inline float calcH0() const noexcept { return -1.0f * xsimd::reduce_add (gCoef.real()); }

        inline void set_freq (float freq)
        {
            const float freqFactor = freq / BBDFilterSpec::outputFilterOriginalCutoff;
            pole_corr = xsimd::exp (poles * (freqFactor * Ts));
            pole_corr_angle = xsimd::arg (pole_corr);

            Amult = gCoef * pole_corr;
        }

        inline void set_time (float tn) noexcept
        {
            Gcalc = Amult * SIMDUtils::pow (pole_corr, 1.0f - tn);
        }

        inline void set_delta (float delta) noexcept { Aplus = BBDFilterSpec::fast_complex_pow (pole_corr_angle, -delta); }

        inline void calcG() noexcept { Gcalc = Aplus * Gcalc; }

        inline void process (const Complex4& u) noexcept { x = pole_corr * x + u; }

        Complex4 x {};
        Complex4 Gcalc { SIMDScalar<T> (1), SIMDScalar<T> (0) };

    private:
        Complex4 gCoef {};
        Complex4 poles {};
        Complex4 pole_corr {};
        xsimd::batch<float> pole_corr_angle {};

        Complex4 Aplus {};

        const float Ts;
        Complex4 Amult {};
    };
} // namespace BBD
} // namespace chowdsp
