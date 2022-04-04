#pragma once

namespace chowdsp::SIMDUtils
{
using namespace SampleTypeHelpers;

template <typename BaseType>
inline SIMDComplex<BaseType> operator+ (const SIMDComplex<BaseType>& a, const SIMDComplex<BaseType>& b)
{
    return { a._r + b._r, a._i + b._i };
}

template <typename BaseType, typename OtherType>
inline std::enable_if_t<std::is_same_v<NumericType<OtherType>, BaseType>, SIMDComplex<BaseType>>
    operator+ (const SIMDComplex<BaseType>& a, OtherType b)
{
    return { a._r + b, a._i };
}

template <typename BaseType, typename OtherType>
inline std::enable_if_t<std::is_same_v<NumericType<OtherType>, BaseType>, SIMDComplex<BaseType>>
    operator+ (OtherType b, const SIMDComplex<BaseType>& a)
{
    return a + b;
}

template <typename BaseType>
inline SIMDComplex<BaseType> operator- (const SIMDComplex<BaseType>& a, const SIMDComplex<BaseType>& b)
{
    return { a._r - b._r, a._i - b._i };
}

template <typename BaseType, typename OtherType>
inline std::enable_if_t<std::is_same_v<NumericType<OtherType>, BaseType>, SIMDComplex<BaseType>>
    operator- (const SIMDComplex<BaseType>& a, OtherType b)
{
    return { a._r - b, a._i };
}

template <typename BaseType, typename OtherType>
inline std::enable_if_t<std::is_same_v<NumericType<OtherType>, BaseType>, SIMDComplex<BaseType>>
    operator- (OtherType b, const SIMDComplex<BaseType>& a)
{
    return { b - a._r, -a._i };
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

template <typename BaseType>
inline SIMDComplex<BaseType> operator* (const SIMDComplex<BaseType>& a, const SIMDComplex<BaseType>& b)
{
    return { SIMDComplexMulReal (a, b), SIMDComplexMulImag (a, b) };
}

template <typename BaseType, typename OtherType>
inline std::enable_if_t<std::is_same_v<NumericType<OtherType>, BaseType>, SIMDComplex<BaseType>>
    operator* (const SIMDComplex<BaseType>& a, OtherType b)
{
    return { a._r * b, a._i * b };
}

template <typename BaseType, typename OtherType>
inline std::enable_if_t<std::is_same_v<NumericType<OtherType>, BaseType>, SIMDComplex<BaseType>>
    operator* (OtherType b, const SIMDComplex<BaseType>& a)
{
    return a * b;
}

/** SIMD implementation of std::conj */
template <typename BaseType>
inline SIMDComplex<BaseType> conj (const SIMDComplex<BaseType>& a)
{
    return { a._r, -a._i };
}

/** Returns the squared absolute value of a SIMDComplex vector */
template <typename BaseType>
inline juce::dsp::SIMDRegister<BaseType> absSquared (const SIMDComplex<BaseType>& a)
{
    return a._r * a._r + a._i * a._i;
}

template <typename BaseType>
inline SIMDComplex<BaseType> operator/ (const SIMDComplex<BaseType>& a, const SIMDComplex<BaseType>& b)
{
    const auto denom = absSquared (b);
    const auto num = a * conj (b);
    return { num._r / denom, num._i / denom };
}

template <typename BaseType, typename OtherType>
inline std::enable_if_t<std::is_same_v<NumericType<OtherType>, BaseType>, SIMDComplex<BaseType>>
    operator/ (const SIMDComplex<BaseType>& a, OtherType b)
{
    return { a._r / b, a._i / b };
}

template <typename BaseType, typename OtherType>
inline std::enable_if_t<std::is_same_v<NumericType<OtherType>, BaseType>, SIMDComplex<BaseType>>
    operator/ (OtherType a, const SIMDComplex<BaseType>& b)
{
    const auto denom = absSquared (b);
    return { a * b._r / denom, -a * b._i / denom };
}

/** SIMDComplex implementation of std::abs */
template <typename BaseType>
inline juce::dsp::SIMDRegister<BaseType> abs (const SIMDComplex<BaseType>& a)
{
    return sqrtSIMD (absSquared (a));
}

/** SIMDComplex implementation of std::arg */
template <typename BaseType>
inline juce::dsp::SIMDRegister<BaseType> arg (const SIMDComplex<BaseType>& a)
{
    return atan2SIMD (a._i, a._r);
}

/** SIMDComplex implementation of std::exp */
template <typename BaseType>
inline SIMDComplex<BaseType> exp (const SIMDComplex<BaseType>& a)
{
    auto isincos = sincosSIMD (a._i);
    return expSIMD (a._r) * SIMDComplex<BaseType> { std::get<1> (isincos), std::get<0> (isincos) };
}

/** SIMDComplex implementation of std::log */
template <typename BaseType>
inline SIMDComplex<BaseType> log (const SIMDComplex<BaseType>& z)
{
    return { logSIMD (abs (z)), arg (z) };
}

/** Ternary select operation for SIMD complex */
template <typename BaseType>
inline SIMDComplex<BaseType> select (vMaskTypeSIMD<BaseType> b, SIMDComplex<BaseType> t, SIMDComplex<BaseType> f)
{
    return { (t._r & b) + (f._r & ~b), (t._i & b) + (f._i & ~b) };
}

/** SIMDComplex implementation of std::pow */
template <typename BaseType>
inline SIMDComplex<BaseType> pow (const SIMDComplex<BaseType>& a, const SIMDComplex<BaseType>& z)
{
    // Reference: https://github.com/xtensor-stack/xsimd/blob/master/include/xsimd/arch/generic/xsimd_generic_math.hpp#L1909
    using VecType = juce::dsp::SIMDRegister<BaseType>;
    auto absa = abs (a);
    auto arga = arg (a);
    auto x = z.real();
    auto y = z.imag();
    auto r = powSIMD (absa, x);
    auto theta = x * arga;

    const auto ze = VecType ((BaseType) 0);
    auto cond = VecType::equal (y, ze);
    r = select (cond, r, r * expSIMD (-y * arga));
    theta = select (cond, theta, theta + y * logSIMD (absa));
    return { r * cosSIMD (theta), r * sinSIMD (theta) };
}

/** SIMDComplex implementation of std::pow */
template <typename BaseType, typename OtherType>
inline std::enable_if_t<std::is_same_v<NumericType<OtherType>, BaseType>, SIMDComplex<BaseType>>
    pow (const SIMDComplex<BaseType>& a, OtherType x)
{
    using VecType = juce::dsp::SIMDRegister<BaseType>;
    auto absa = abs (a);
    auto arga = arg (a);
    auto r = powSIMD (absa, (VecType) x);
    auto theta = x * arga;
    return { r * cosSIMD (theta), r * sinSIMD (theta) };
}

/** SIMDComplex implementation of std::pow */
template <typename BaseType, typename OtherType>
inline std::enable_if_t<std::is_same_v<NumericType<OtherType>, BaseType>, SIMDComplex<BaseType>>
    pow (OtherType a, const SIMDComplex<BaseType>& z)
{
    // @TODO: there's probably some optimization we can do here, knowing that the base is real?
    using VecType = juce::dsp::SIMDRegister<BaseType>;
    return pow (SIMDComplex<BaseType> { a, VecType {} }, z);
}

template <typename BaseType>
inline vMaskTypeSIMD<BaseType> operator== (const SIMDComplex<BaseType>& a, const SIMDComplex<BaseType>& b)
{
    using VecType = juce::dsp::SIMDRegister<BaseType>;
    return VecType::equal (a._r, b._r) & VecType::equal (a._i, b._i);
}

template <typename BaseType>
inline vMaskTypeSIMD<BaseType> operator!= (const SIMDComplex<BaseType>& a, const SIMDComplex<BaseType>& b)
{
    using VecType = juce::dsp::SIMDRegister<BaseType>;
    return VecType::notEqual (a._r, b._r) | VecType::notEqual (a._i, b._i);
}
} // namespace chowdsp::SIMDUtils
