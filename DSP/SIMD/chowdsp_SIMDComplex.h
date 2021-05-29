#pragma once

namespace chowdsp
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

    constexpr SIMDComplex (const T& r = T (0), const T& i = T (0))
        : _r (r), _i (i)
    {
    }

    constexpr SIMDComplex (Type r, Type i)
        : _r (T (r)), _i (T (i))
    {
    }

    constexpr SIMDComplex (Type r[size], Type i[size])
    {
        _r = T::fromRawArray (r);
        _i = T::fromRawArray (i);
    }

    SIMDComplex<Type> (std::initializer_list<Type> r, std::initializer_list<Type> i)
    {
        if (r.size() != size && i.size() != size)
        {
            throw std::invalid_argument ("Initialize lists must be of size 4");
        }
        Type rfl alignas (16)[size], ifl alignas (16)[size];
        for (size_t q = 0; q < size; ++q)
        {
            rfl[q] = *(r.begin() + q);
            ifl[q] = *(i.begin() + q);
        }

        _r = T::fromRawArray (rfl);
        _i = T::fromRawArray (ifl);
    }

    inline T real() const noexcept { return _r; }
    inline T imag() const noexcept { return _i; }

    inline SIMDComplex<Type>& operator+= (const SIMDComplex<Type>& o)
    {
        _r = _r + o._r;
        _i = _i + o._i;
        return *this;
    }

    std::complex<Type> atIndex (size_t idx) const
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

    inline SIMDComplex<Type> map (std::function<std::complex<Type> (const std::complex<Type>&)> f)
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

    inline juce::dsp::SIMDRegister<Type> map_float (std::function<Type (const std::complex<Type>&)> f)
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

template <typename Type>
inline SIMDComplex<Type> operator+ (const SIMDComplex<Type>& a, const SIMDComplex<Type>& b)
{
    return { a._r + b._r, a._i + b._i };
}

template <typename Type>
inline juce::dsp::SIMDRegister<Type> SIMDComplexMulReal (const SIMDComplex<Type>& a, const SIMDComplex<Type>& b)
{
    return (a._r * b._r) - (a._i * b._i);
}

template <typename Type>
inline juce::dsp::SIMDRegister<Type> SIMDComplexMulImag (const SIMDComplex<Type>& a, const SIMDComplex<Type>& b)
{
    return (a._r * b._i) + (a._i * b._r);
}

template <typename Type>
inline SIMDComplex<Type> operator* (const SIMDComplex<Type>& a, const SIMDComplex<Type>& b)
{
    return { SIMDComplexMulReal (a, b), SIMDComplexMulImag (a, b) };
}

template <typename Type>
inline SIMDComplex<Type> operator* (const SIMDComplex<Type>& a, const Type& b)
{
    return { a._r * b, a._i * b };
}

template <typename Type>
inline SIMDComplex<Type> operator* (const SIMDComplex<Type>& a, const juce::dsp::SIMDRegister<Type>& b)
{
    return { a._r * b, a._i * b };
}

template <typename Type>
inline SIMDComplex<Type> operator* (const juce::dsp::SIMDRegister<Type>& b, const SIMDComplex<Type>& a)
{
    return { a._r * b, a._i * b };
}

} // namespace chowdsp
