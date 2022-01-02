#pragma once

/**
 * Bucket-Bridage Device emulation, as derived by
 * Martin Holters and Julian Parker:
 * http://dafx2018.web.ua.pt/papers/DAFx2018_paper_12.pdf
 */
namespace chowdsp::BBD
{
template <typename T>
using SIMDComplex = SIMDUtils::SIMDComplex<T>;

/**
 * Anti-aliasing/reconstruction filters used by JUNO-60 chorus.
 * Root/Pole analysis borrowed from the above paper.
 */
namespace BBDFilterSpec
{
    constexpr size_t N_filt = 4;

    constexpr std::complex<float> iFiltRoot[] = { { -10329.2715f, -329.848f },
                                                  { -10329.2715f, +329.848f },
                                                  { 366.990557f, -1811.4318f },
                                                  { 366.990557f, +1811.4318f } };
    constexpr std::complex<float> iFiltPole[] = {
        { -55482.0f, -25082.0f },
        { -55482.0f, +25082.0f },
        { -26292.0f, -59437.0f },
        { -26292.0f, +59437.0f }
    };

    constexpr std::complex<float> oFiltRoot[] = {
        { -11256.0f, -99566.0f },
        { -11256.0f, +99566.0f },
        { -13802.0f, -24606.0f },
        { -13802.0f, +24606.0f }
    };
    constexpr std::complex<float> oFiltPole[] = {
        { -51468.0f, -21437.0f },
        { -51468.0f, +21437.0f },
        { -26276.0f, -59699.0f },
        { -26276.0f, +59699.0f }
    };

    inline SIMDComplex<float> fast_complex_pow (juce::dsp::SIMDRegister<float> angle, float b)
    {
        auto angle_pow = angle * b;
        return SIMDComplex<float>::fastExp (angle_pow);
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
        float root_real alignas (16)[4];
        float root_imag alignas (16)[4];
        float pole_real alignas (16)[4];
        float pole_imag alignas (16)[4];
        for (size_t i = 0; i < BBDFilterSpec::N_filt; ++i)
        {
            root_real[i] = BBDFilterSpec::iFiltRoot[i].real();
            root_imag[i] = BBDFilterSpec::iFiltRoot[i].imag();

            pole_real[i] = BBDFilterSpec::iFiltPole[i].real();
            pole_imag[i] = BBDFilterSpec::iFiltPole[i].imag();
        }
        roots = Complex4 (root_real, root_imag);
        poles = Complex4 (pole_real, pole_imag);
    }

    inline void set_freq (float freq)
    {
        constexpr float originalCutoff = 9900.0f;
        const float freqFactor = freq / originalCutoff;
        root_corr = roots * freqFactor;
        pole_corr = poles.map ([&freqFactor, this] (const std::complex<float>& f) { return std::exp (f * freqFactor * Ts); });

        pole_corr_angle =
            pole_corr.map_float ([] (const std::complex<float>& f) { return std::arg (f); });

        gCoef = root_corr * Ts;
    }

    inline void set_time (float tn) noexcept
    {
        Gcalc =
            gCoef * pole_corr.map ([&tn] (const std::complex<float>& f) { return std::pow (f, tn); });
    }

    inline void set_delta (float delta) noexcept
    {
        Aplus = BBDFilterSpec::fast_complex_pow (pole_corr_angle, delta);
    }

    inline void calcG() noexcept { Gcalc = Aplus * Gcalc; }

    inline void process (float u) noexcept
    {
        x = pole_corr * x + Complex4 (u, 0.0f);
    }

    Complex4 x;
    Complex4 Gcalc { (T) 1, (T) 0 };

private:
    Complex4 roots;
    Complex4 poles;
    Complex4 root_corr;
    Complex4 pole_corr;
    juce::dsp::SIMDRegister<T> pole_corr_angle {};

    Complex4 Aplus;

    const T Ts;
    Complex4 gCoef;
};

/** Filter bank for BBD signal output. */
class OutputFilterBank
{
    using T = float; // We need SIMD size 4 (for now)
    using Complex4 = SIMDComplex<T>;

public:
    explicit OutputFilterBank (float sampleTime) : Ts (sampleTime)
    {
        float gcoefs_real alignas (16)[4];
        float gcoefs_imag alignas (16)[4];
        float pole_real alignas (16)[4];
        float pole_imag alignas (16)[4];
        for (size_t i = 0; i < BBDFilterSpec::N_filt; ++i)
        {
            auto gVal = BBDFilterSpec::oFiltRoot[i] / BBDFilterSpec::oFiltPole[i];
            gcoefs_real[i] = gVal.real();
            gcoefs_imag[i] = gVal.imag();

            pole_real[i] = BBDFilterSpec::oFiltPole[i].real();
            pole_imag[i] = BBDFilterSpec::oFiltPole[i].imag();
        }
        gCoef = Complex4 (gcoefs_real, gcoefs_imag);
        poles = Complex4 (pole_real, pole_imag);
    }

    inline float calcH0() const noexcept { return -1.0f * gCoef.real().sum(); }

    inline void set_freq (float freq)
    {
        constexpr float originalCutoff = 9500.0f;
        const float freqFactor = freq / originalCutoff;
        pole_corr = poles.map ([&freqFactor, this] (const std::complex<float>& f) { return std::exp (f * freqFactor * Ts); });

        pole_corr_angle =
            pole_corr.map_float ([] (const std::complex<float>& f) { return std::arg (f); });

        Amult = gCoef * pole_corr;
    }

    inline void set_time (float tn) noexcept
    {
        Gcalc = Amult * pole_corr.map ([&tn] (const std::complex<float>& f) { return std::pow (f, 1.0f - tn); });
    }

    inline void set_delta (float delta) noexcept { Aplus = BBDFilterSpec::fast_complex_pow (pole_corr_angle, -delta); }

    inline void calcG() noexcept { Gcalc = Aplus * Gcalc; }

    inline void process (Complex4 u) noexcept { x = pole_corr * x + u; }

    Complex4 x;
    Complex4 Gcalc { 1.0f, 0.0f };

private:
    Complex4 gCoef;
    Complex4 poles;
    Complex4 root_corr;
    Complex4 pole_corr;
    juce::dsp::SIMDRegister<float> pole_corr_angle {};

    Complex4 Aplus;

    const float Ts;
    Complex4 Amult;
};

} // namespace chowdsp::BBD
