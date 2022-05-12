#pragma once

namespace chowdsp::SIMDUtils
{
using namespace SampleTypeHelpers;

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
} // namespace chowdsp::SIMDUtils
