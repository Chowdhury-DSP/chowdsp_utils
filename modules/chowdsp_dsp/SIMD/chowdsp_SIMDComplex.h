#pragma once

namespace chowdsp::SIMDUtils
{
/**
 * Implementation of complex numbers using juce::dsp::SIMDRegister,
 * along with a few helpful operators.
 */
template <typename Type>
struct SIMDComplex
{
    using T = juce::dsp::SIMDRegister<Type>;
    T _r, _i;
    static constexpr size_t size = T::size();

    constexpr SIMDComplex (const T& r = T (0), const T& i = T (0)) //NOLINT(google-explicit-constructor) we want to be able to use this constructor implicitly (see math ops at the bottom of this file)
        : _r (r), _i (i)
    {
    }

    constexpr SIMDComplex (Type r, Type i)
        : _r (T (r)), _i (T (i))
    {
    }

    constexpr SIMDComplex (std::complex<Type> c) //NOLINT(google-explicit-constructor) we want to be able to use this constructor implicitly (see math ops at the bottom of this file)
        : _r (T (c.real())), _i (T (c.imag()))
    {
    }

    SIMDComplex<Type> (const Type (&r)[size], const Type (&i)[size])
    {
        _r = SIMDUtils::loadUnaligned (r);
        _i = SIMDUtils::loadUnaligned (i);
    }

    [[nodiscard]] inline T real() const noexcept { return _r; }
    [[nodiscard]] inline T imag() const noexcept { return _i; }

    inline SIMDComplex<Type>& operator+= (const SIMDComplex<Type>& o)
    {
        _r = _r + o._r;
        _i = _i + o._i;
        return *this;
    }

    [[nodiscard]] std::complex<Type> atIndex (size_t idx) const
    {
        return std::complex<Type> { _r.get (idx), _i.get (idx) };
    }

    inline static SIMDComplex fastExp (T angle)
    {
        using namespace SIMDUtils;
        angle = clampToPiRangeSIMD<Type> (angle);
        return { fastcosSIMD<Type> (angle), fastsinSIMD<Type> (angle) };
    }

    inline static SIMDComplex exp (T angle)
    {
        using namespace SIMDUtils;
        return { cosSIMD (angle), sinSIMD (angle) };
    }

    inline static SIMDComplex polar (T mag, T angle)
    {
        return mag * exp (angle);
    }

    inline SIMDComplex<Type> map (std::function<std::complex<Type> (const std::complex<Type>&)> f) const noexcept
    {
        Type rfl alignas (16)[size], ifl alignas (16)[size];
        _r.copyToRawArray (rfl);
        _i.copyToRawArray (ifl);

        Type rflR alignas (16)[size], iflR alignas (16)[size];
        for (size_t i = 0; i < size; ++i)
        {
            auto a = std::complex<Type> { rfl[i], ifl[i] };
            auto b = f (a);
            rflR[i] = b.real();
            iflR[i] = b.imag();
        }
        return { T::fromRawArray (rflR), T::fromRawArray (iflR) };
    }

    inline juce::dsp::SIMDRegister<Type> map_float (std::function<Type (const std::complex<Type>&)> f) const noexcept
    {
        Type rfl alignas (16)[size], ifl alignas (16)[size];
        _r.copyToRawArray (rfl);
        _i.copyToRawArray (ifl);

        Type out alignas (16)[size];
        for (size_t i = 0; i < size; ++i)
        {
            auto a = std::complex<Type> { rfl[i], ifl[i] };
            out[i] = f (a);
        }
        return T::fromRawArray (out);
    }
};
} // namespace chowdsp::SIMDUtils
