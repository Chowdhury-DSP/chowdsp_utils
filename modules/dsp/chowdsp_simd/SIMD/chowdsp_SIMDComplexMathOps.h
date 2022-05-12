#pragma once

namespace chowdsp::SIMDUtils
{
using namespace SampleTypeHelpers;

//template <typename BaseType>
//inline SIMDComplex<BaseType> operator+ (const SIMDComplex<BaseType>& a, const SIMDComplex<BaseType>& b)
//{
//    return { a._r + b._r, a._i + b._i };
//}
//
//template <typename BaseType, typename OtherType>
//inline std::enable_if_t<std::is_same_v<NumericType<OtherType>, BaseType>, SIMDComplex<BaseType>>
//    operator+ (const SIMDComplex<BaseType>& a, OtherType b)
//{
//    return { a._r + b, a._i };
//}
//
//template <typename BaseType, typename OtherType>
//inline std::enable_if_t<std::is_same_v<NumericType<OtherType>, BaseType>, SIMDComplex<BaseType>>
//    operator+ (OtherType b, const SIMDComplex<BaseType>& a)
//{
//    return a + b;
//}
//
//template <typename BaseType>
//inline SIMDComplex<BaseType> operator- (const SIMDComplex<BaseType>& a, const SIMDComplex<BaseType>& b)
//{
//    return { a._r - b._r, a._i - b._i };
//}
//
//template <typename BaseType, typename OtherType>
//inline std::enable_if_t<std::is_same_v<NumericType<OtherType>, BaseType>, SIMDComplex<BaseType>>
//    operator- (const SIMDComplex<BaseType>& a, OtherType b)
//{
//    return { a._r - b, a._i };
//}
//
//template <typename BaseType, typename OtherType>
//inline std::enable_if_t<std::is_same_v<NumericType<OtherType>, BaseType>, SIMDComplex<BaseType>>
//    operator- (OtherType b, const SIMDComplex<BaseType>& a)
//{
//    return { b - a._r, -a._i };
//}

template <typename Type>
inline juce::dsp::SIMDRegister<Type> SIMDComplexMulReal (const xsimd::batch<std::complex<Type>>& a, const xsimd::batch<std::complex<Type>>& b)
{
    return juce::dsp::SIMDRegister<Type> ((a.real() * b.real()) - (a.imag() * b.imag()));
}

template <typename Type>
inline juce::dsp::SIMDRegister<Type> SIMDComplexMulImag (const xsimd::batch<std::complex<Type>>& a, const xsimd::batch<std::complex<Type>>& b)
{
    return juce::dsp::SIMDRegister<Type> ((a.real() * b.imag()) + (a.imag() * b.real()));
}

//template <typename BaseType>
//inline SIMDComplex<BaseType> operator* (const SIMDComplex<BaseType>& a, const SIMDComplex<BaseType>& b)
//{
//    return { SIMDComplexMulReal (a, b), SIMDComplexMulImag (a, b) };
//}
//
//template <typename BaseType, typename OtherType>
//inline std::enable_if_t<std::is_same_v<NumericType<OtherType>, BaseType>, SIMDComplex<BaseType>>
//    operator* (const SIMDComplex<BaseType>& a, OtherType b)
//{
//    return { a._r * b, a._i * b };
//}
//
//template <typename BaseType, typename OtherType>
//inline std::enable_if_t<std::is_same_v<NumericType<OtherType>, BaseType>, SIMDComplex<BaseType>>
//    operator* (OtherType b, const SIMDComplex<BaseType>& a)
//{
//    return a * b;
//}
//
///** SIMD implementation of std::conj */
//template <typename BaseType>
//inline SIMDComplex<BaseType> conj (const SIMDComplex<BaseType>& a)
//{
//    return { a._r, -a._i };
//}
//
///** Returns the squared absolute value of a SIMDComplex vector */
//template <typename BaseType>
//inline juce::dsp::SIMDRegister<BaseType> absSquared (const SIMDComplex<BaseType>& a)
//{
//    return a._r * a._r + a._i * a._i;
//}
//
//template <typename BaseType>
//inline SIMDComplex<BaseType> operator/ (const SIMDComplex<BaseType>& a, const SIMDComplex<BaseType>& b)
//{
//    const auto denom = absSquared (b);
//    const auto num = a * conj (b);
//    return { num._r / denom, num._i / denom };
//}
//
//template <typename BaseType, typename OtherType>
//inline std::enable_if_t<std::is_same_v<NumericType<OtherType>, BaseType>, SIMDComplex<BaseType>>
//    operator/ (const SIMDComplex<BaseType>& a, OtherType b)
//{
//    return { a._r / b, a._i / b };
//}
//
//template <typename BaseType, typename OtherType>
//inline std::enable_if_t<std::is_same_v<NumericType<OtherType>, BaseType>, SIMDComplex<BaseType>>
//    operator/ (OtherType a, const SIMDComplex<BaseType>& b)
//{
//    const auto denom = absSquared (b);
//    return { a * b._r / denom, -a * b._i / denom };
//}
//
///** SIMDComplex implementation of std::abs */
//template <typename BaseType>
//inline juce::dsp::SIMDRegister<BaseType> abs (const SIMDComplex<BaseType>& a)
//{
//    return sqrtSIMD (absSquared (a));
//}
//
///** SIMDComplex implementation of std::arg */
//template <typename BaseType>
//inline juce::dsp::SIMDRegister<BaseType> arg (const SIMDComplex<BaseType>& a)
//{
//    return atan2SIMD (a._i, a._r);
//}
//
///** SIMDComplex implementation of std::exp */
//template <typename BaseType>
//inline SIMDComplex<BaseType> exp (const SIMDComplex<BaseType>& a)
//{
//    auto isincos = sincosSIMD (a._i);
//    return expSIMD (a._r) * SIMDComplex<BaseType> { std::get<1> (isincos), std::get<0> (isincos) };
//}
//
///** SIMDComplex implementation of std::log */
//template <typename BaseType>
//inline SIMDComplex<BaseType> log (const SIMDComplex<BaseType>& z)
//{
//    return { logSIMD (abs (z)), arg (z) };
//}
//
///** Ternary select operation for SIMD complex */
//template <typename BaseType>
//inline SIMDComplex<BaseType> select (vMaskTypeSIMD<BaseType> b, SIMDComplex<BaseType> t, SIMDComplex<BaseType> f)
//{
//    return { (t._r & b) + (f._r & ~b), (t._i & b) + (f._i & ~b) };
//}
//
///** SIMDComplex implementation of std::pow */
//template <typename BaseType>
//inline SIMDComplex<BaseType> pow (const SIMDComplex<BaseType>& a, const SIMDComplex<BaseType>& z)
//{
//    // Reference: https://github.com/xtensor-stack/xsimd/blob/master/include/xsimd/arch/generic/xsimd_generic_math.hpp#L1909
//    using VecType = juce::dsp::SIMDRegister<BaseType>;
//    auto absa = abs (a);
//    auto arga = arg (a);
//    auto x = z.real();
//    auto y = z.imag();
//    auto r = powSIMD (absa, x);
//    auto theta = x * arga;
//
//    const auto ze = VecType ((BaseType) 0);
//    auto cond = VecType::equal (y, ze);
//    r = select (cond, r, r * expSIMD (-y * arga));
//    theta = select (cond, theta, theta + y * logSIMD (absa));
//    auto sincosTheta = sincosSIMD (theta);
//    return { r * sincosTheta.second, r * sincosTheta.first };
//}

// xsimd doesn't have these implementations (yet)

/** SIMDComplex implementation of std::pow */
template <typename BaseType, typename OtherType>
inline std::enable_if_t<std::is_same_v<NumericType<OtherType>, BaseType>, xsimd::batch<std::complex<BaseType>>>
    pow (const xsimd::batch<std::complex<BaseType>>& a, OtherType x)
{
    auto absa = xsimd::abs (a);
    auto arga = xsimd::arg (a);
    auto r = xsimd::pow (absa, xsimd::batch (x));
    auto theta = x * arga;
    auto sincosTheta = xsimd::sincos (theta);
    return { r * sincosTheta.second, r * sincosTheta.first };
}

/** SIMDComplex implementation of std::pow */
template <typename BaseType, typename OtherType>
inline std::enable_if_t<std::is_same_v<NumericType<OtherType>, BaseType>, xsimd::batch<std::complex<BaseType>>>
    pow (OtherType a, const xsimd::batch<std::complex<BaseType>>& z)
{
    // same as the complex/complex xsimd implementation, except that we can skip calling arg()!
    const auto ze = xsimd::batch ((BaseType) 0);

    auto absa = xsimd::abs (a);
    auto arga = xsimd::select (a >= ze, ze, xsimd::batch (juce::MathConstants<BaseType>::pi)); // since a is real, we know arg must be either 0 or pi
    auto x = z.real();
    auto y = z.imag();
    auto r = xsimd::pow (absa, x);
    auto theta = x * arga;

    auto cond = y == ze;
    r = select (cond, r, r * xsimd::exp (-y * arga));
    theta = select (cond, theta, theta + y * xsimd::log (absa));
    auto sincosTheta = xsimd::sincos (theta);
    return { r * sincosTheta.second, r * sincosTheta.first };
}

template <typename BaseType>
inline xsimd::batch<std::complex<BaseType>> polar (const juce::dsp::SIMDRegister<BaseType>& mag, const juce::dsp::SIMDRegister<BaseType>& angle)
{
    const auto r = xsimd::batch<BaseType> (mag.value);
    auto sincosTheta = xsimd::sincos (xsimd::batch<BaseType> (angle.value));
    return { r * sincosTheta.second, r * sincosTheta.first };
}

template <typename BaseType>
inline static xsimd::batch<std::complex<BaseType>> polar (const juce::dsp::SIMDRegister<BaseType>& angle)
{
    auto sincosTheta = xsimd::sincos (xsimd::batch<BaseType> (angle.value));
    return { sincosTheta.second, sincosTheta.first };
}

//template <typename BaseType>
//inline vMaskTypeSIMD<BaseType> operator== (const SIMDComplex<BaseType>& a, const SIMDComplex<BaseType>& b)
//{
//    using VecType = juce::dsp::SIMDRegister<BaseType>;
//    return VecType::equal (a._r, b._r) & VecType::equal (a._i, b._i);
//}
//
//template <typename BaseType>
//inline vMaskTypeSIMD<BaseType> operator!= (const SIMDComplex<BaseType>& a, const SIMDComplex<BaseType>& b)
//{
//    using VecType = juce::dsp::SIMDRegister<BaseType>;
//    return VecType::notEqual (a._r, b._r) | VecType::notEqual (a._i, b._i);
//}
} // namespace chowdsp::SIMDUtils
