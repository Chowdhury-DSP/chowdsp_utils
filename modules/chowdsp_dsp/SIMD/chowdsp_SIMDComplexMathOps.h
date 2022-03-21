#pragma once

namespace chowdsp::SIMDUtils
{
template <typename BaseType>
inline SIMDComplex<BaseType> operator+ (const SIMDComplex<BaseType>& a, const SIMDComplex<BaseType>& b)
{
    return { a._r + b._r, a._i + b._i };
}

template <typename BaseType, typename OtherType>
inline typename std::enable_if<std::is_same<OtherType, BaseType>::value || std::is_same<OtherType, juce::dsp::SIMDRegister<BaseType>>::value,
                               SIMDComplex<BaseType>>::type operator+ (const SIMDComplex<BaseType>& a, OtherType b)
{
    return { a._r + b, a._i };
}

template <typename BaseType, typename OtherType>
inline typename std::enable_if<std::is_same<OtherType, BaseType>::value || std::is_same<OtherType, juce::dsp::SIMDRegister<BaseType>>::value,
                               SIMDComplex<BaseType>>::type operator+ (OtherType b, const SIMDComplex<BaseType>& a)
{
    return a + b;
}

template <typename BaseType>
inline SIMDComplex<BaseType> operator- (const SIMDComplex<BaseType>& a, const SIMDComplex<BaseType>& b)
{
    return { a._r - b._r, a._i - b._i };
}

template <typename BaseType, typename OtherType>
inline typename std::enable_if<std::is_same<OtherType, BaseType>::value || std::is_same<OtherType, juce::dsp::SIMDRegister<BaseType>>::value,
                               SIMDComplex<BaseType>>::type operator- (const SIMDComplex<BaseType>& a, OtherType b)
{
    return { a._r - b, a._i };
}

template <typename BaseType, typename OtherType>
inline typename std::enable_if<std::is_same<OtherType, BaseType>::value || std::is_same<OtherType, juce::dsp::SIMDRegister<BaseType>>::value,
                               SIMDComplex<BaseType>>::type operator- (OtherType b, const SIMDComplex<BaseType>& a)
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
inline typename std::enable_if<std::is_same<OtherType, BaseType>::value || std::is_same<OtherType, juce::dsp::SIMDRegister<BaseType>>::value,
                               SIMDComplex<BaseType>>::type operator* (const SIMDComplex<BaseType>& a, OtherType b)
{
    return { a._r * b, a._i * b };
}

template <typename BaseType, typename OtherType>
inline typename std::enable_if<std::is_same<OtherType, BaseType>::value || std::is_same<OtherType, juce::dsp::SIMDRegister<BaseType>>::value,
                               SIMDComplex<BaseType>>::type operator* (OtherType b, const SIMDComplex<BaseType>& a)
{
    return a * b;
}

template <typename BaseType>
inline SIMDComplex<BaseType> conj (const SIMDComplex<BaseType>& a)
{
    return { a._r, -a._i };
}

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
inline typename std::enable_if<std::is_same<OtherType, BaseType>::value || std::is_same<OtherType, juce::dsp::SIMDRegister<BaseType>>::value,
                               SIMDComplex<BaseType>>::type operator/ (const SIMDComplex<BaseType>& a, OtherType b)
{
    return { a._r / b, a._i / b };
}

template <typename BaseType, typename OtherType>
inline typename std::enable_if<std::is_same<OtherType, BaseType>::value || std::is_same<OtherType, juce::dsp::SIMDRegister<BaseType>>::value,
                               SIMDComplex<BaseType>>::type operator/ (OtherType a, const SIMDComplex<BaseType>& b)
{
    const auto denom = absSquared (b);
    return { a * b._r / denom, -a * b._i / denom };
}
} // namespace chowdsp::SIMDUtils
