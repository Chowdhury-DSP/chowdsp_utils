#pragma once

namespace chowdsp::SIMDUtils
{
/**
 *  JUCE SIMD registers don't directly support more compilcated math functions
 *  We can use xsimd to enable this if xsimd is available, otherwise,
 *  let's implement a fallback.
 */

#if CHOWDSP_USE_XSIMD // XSIMD implementations
template <typename T>
using x_type = xsimd::batch<T, juce::dsp::SIMDRegister<T>::size()>;

/** SIMD implementation of std::exp */
template <typename T>
inline juce::dsp::SIMDRegister<T> expSIMD (juce::dsp::SIMDRegister<T> x)
{
    return (juce::dsp::SIMDRegister<T>) xsimd::exp ((x_type<T>) x.value);
}

/** SIMD implementation of std::log */
template <typename T>
inline juce::dsp::SIMDRegister<T> logSIMD (juce::dsp::SIMDRegister<T> x)
{
    return (juce::dsp::SIMDRegister<T>) xsimd::log ((x_type<T>) x.value);
}

/** SIMD implementation of std::log10 */
template <typename T>
inline juce::dsp::SIMDRegister<T> log10SIMD (juce::dsp::SIMDRegister<T> x)
{
    return (juce::dsp::SIMDRegister<T>) xsimd::log10 ((x_type<T>) x.value);
}

/** SIMD implementation of std::pow */
template <typename T>
inline juce::dsp::SIMDRegister<T> powSIMD (juce::dsp::SIMDRegister<T> a, juce::dsp::SIMDRegister<T> b)
{
    return (juce::dsp::SIMDRegister<T>) xsimd::pow ((x_type<T>) a.value, (x_type<T>) b.value);
}

/** SIMD implementation of std::sqrt */
template <typename T>
inline juce::dsp::SIMDRegister<T> sqrtSIMD (juce::dsp::SIMDRegister<T> x)
{
    return (juce::dsp::SIMDRegister<T>) xsimd::sqrt ((x_type<T>) x.value);
}

/** SIMD implementation of std::sin */
template <typename T>
inline juce::dsp::SIMDRegister<T> sinSIMD (juce::dsp::SIMDRegister<T> x)
{
    return (juce::dsp::SIMDRegister<T>) xsimd::sin ((x_type<T>) x.value);
}

/** SIMD implementation of std::cos */
template <typename T>
inline juce::dsp::SIMDRegister<T> cosSIMD (juce::dsp::SIMDRegister<T> x)
{
    return (juce::dsp::SIMDRegister<T>) xsimd::cos ((x_type<T>) x.value);
}

/** SIMD implementation of std::tan */
template <typename T>
inline juce::dsp::SIMDRegister<T> tanSIMD (juce::dsp::SIMDRegister<T> x)
{
    return (juce::dsp::SIMDRegister<T>) xsimd::tan ((x_type<T>) x.value);
}

/** SIMD implementation of std::sinh */
template <typename T>
inline juce::dsp::SIMDRegister<T> sinhSIMD (juce::dsp::SIMDRegister<T> x)
{
    return (juce::dsp::SIMDRegister<T>) xsimd::sinh ((x_type<T>) x.value);
}

/** SIMD implementation of std::cosh */
template <typename T>
inline juce::dsp::SIMDRegister<T> coshSIMD (juce::dsp::SIMDRegister<T> x)
{
    return (juce::dsp::SIMDRegister<T>) xsimd::cosh ((x_type<T>) x.value);
}

/** SIMD implementation of std::tanh */
template <typename T>
inline juce::dsp::SIMDRegister<T> tanhSIMD (juce::dsp::SIMDRegister<T> x)
{
    return (juce::dsp::SIMDRegister<T>) xsimd::tanh ((x_type<T>) x.value);
}

/** SIMD implementation of std::isnan */
template <typename T>
inline typename juce::dsp::SIMDRegister<T>::vMaskType isnanSIMD (juce::dsp::SIMDRegister<T> x)
{
    // For some reason, xsimd::isnan returns a batch of doubles when using SSE
    // but returns a batch of unsigned ints when using ARM NEON.
#if JUCE_ARM
    using MaskVec = typename juce::dsp::SIMDRegister<T>::vMaskType;
    return (MaskVec) xsimd::isnan ((x_type<T>) x.value);
#else
    using Vec = juce::dsp::SIMDRegister<T>;
    return Vec::notEqual ((Vec) xsimd::isnan ((x_type<T>) x.value), (Vec) 0);
#endif
}

// Template specializations for NEON double precision
#if (! CHOWDSP_USE_CUSTOM_JUCE_DSP) && (defined(_M_ARM64) || defined(__arm64__) || defined(__aarch64__))
// We need to specialize the double versions of the functions
/** SIMD implementation of std::exp */
template <>
inline juce::dsp::SIMDRegister<double> expSIMD (juce::dsp::SIMDRegister<double> x)
{
    auto y = juce::dsp::SIMDRegister<double> ((double) 0);
    for (size_t i = 0; i < x.size(); ++i)
        y.set (i, std::exp (x.get (i)));

    return y;
}

/** SIMD implementation of std::log */
template <>
inline juce::dsp::SIMDRegister<double> logSIMD (juce::dsp::SIMDRegister<double> x)
{
    auto y = juce::dsp::SIMDRegister<double> ((double) 0);
    for (size_t i = 0; i < x.size(); ++i)
        y.set (i, std::log (x.get (i)));

    return y;
}

/** SIMD implementation of std::log */
template <>
inline juce::dsp::SIMDRegister<double> log10SIMD (juce::dsp::SIMDRegister<double> x)
{
    auto y = juce::dsp::SIMDRegister<double> ((double) 0);
    for (size_t i = 0; i < x.size(); ++i)
        y.set (i, std::log10 (x.get (i)));

    return y;
}

/** SIMD implementation of std::pow */
template <>
inline juce::dsp::SIMDRegister<double> powSIMD (juce::dsp::SIMDRegister<double> a, juce::dsp::SIMDRegister<double> b)
{
    auto y = juce::dsp::SIMDRegister<double> ((double) 0);
    for (size_t i = 0; i < a.size(); ++i)
        y.set (i, std::pow (a.get (i), b.get (i)));

    return y;
}

/** SIMD implementation of std::sqrt */
template <>
inline juce::dsp::SIMDRegister<double> sqrtSIMD (juce::dsp::SIMDRegister<double> x)
{
    auto y = juce::dsp::SIMDRegister<double> ((double) 0);
    for (size_t i = 0; i < x.size(); ++i)
        y.set (i, std::sqrt (x.get (i)));

    return y;
}

/** SIMD implementation of std::sin */
template <>
inline juce::dsp::SIMDRegister<double> sinSIMD (juce::dsp::SIMDRegister<double> x)
{
    auto y = juce::dsp::SIMDRegister<double> ((double) 0);
    for (size_t i = 0; i < x.size(); ++i)
        y.set (i, std::sin (x.get (i)));

    return y;
}

/** SIMD implementation of std::cos */
template <>
inline juce::dsp::SIMDRegister<double> cosSIMD (juce::dsp::SIMDRegister<double> x)
{
    auto y = juce::dsp::SIMDRegister<double> ((double) 0);
    for (size_t i = 0; i < x.size(); ++i)
        y.set (i, std::cos (x.get (i)));

    return y;
}

/** SIMD implementation of std::tan */
template <>
inline juce::dsp::SIMDRegister<double> tanSIMD (juce::dsp::SIMDRegister<double> x)
{
    auto y = juce::dsp::SIMDRegister<double> ((double) 0);
    for (size_t i = 0; i < x.size(); ++i)
        y.set (i, std::tan (x.get (i)));

    return y;
}

/** SIMD implementation of std::sinh */
template <>
inline juce::dsp::SIMDRegister<double> sinhSIMD (juce::dsp::SIMDRegister<double> x)
{
    auto y = juce::dsp::SIMDRegister<double> ((double) 0);
    for (size_t i = 0; i < x.size(); ++i)
        y.set (i, std::sinh (x.get (i)));

    return y;
}

/** SIMD implementation of std::cosh */
template <>
inline juce::dsp::SIMDRegister<double> coshSIMD (juce::dsp::SIMDRegister<double> x)
{
    auto y = juce::dsp::SIMDRegister<double> ((double) 0);
    for (size_t i = 0; i < x.size(); ++i)
        y.set (i, std::cosh (x.get (i)));

    return y;
}

/** SIMD implementation of std::tanh */
template <>
inline juce::dsp::SIMDRegister<double> tanhSIMD (juce::dsp::SIMDRegister<double> x)
{
    auto y = juce::dsp::SIMDRegister<double> ((double) 0);
    for (size_t i = 0; i < x.size(); ++i)
        y.set (i, std::tanh (x.get (i)));

    return y;
}

/** SIMD implementation of std::isnan */
template <>
inline juce::dsp::SIMDRegister<double>::vMaskType isnanSIMD (juce::dsp::SIMDRegister<double> x)
{
    auto y = juce::dsp::SIMDRegister<double>::vMaskType();
    for (size_t i = 0; i < x.size(); ++i)
        y.set (i, std::isnan (x.get (i)));

    return y;
}
#endif // (! CHOWDSP_USE_CUSTOM_JUCE_DSP) && (defined(_M_ARM64) || defined(__arm64__) || defined(__aarch64__))

#else // fallback implemetations (! CHOWDSP_USE_XSIMD)
/** SIMD implementation of std::exp */
template <typename T>
inline juce::dsp::SIMDRegister<T> expSIMD (juce::dsp::SIMDRegister<T> x)
{
    auto y = juce::dsp::SIMDRegister<T> ((T) 0);
    for (size_t i = 0; i < x.size(); ++i)
        y.set (i, std::exp (x.get (i)));

    return y;
}

/** SIMD implementation of std::log */
template <typename T>
inline juce::dsp::SIMDRegister<T> logSIMD (juce::dsp::SIMDRegister<T> x)
{
    auto y = juce::dsp::SIMDRegister<T> ((T) 0);
    for (size_t i = 0; i < x.size(); ++i)
        y.set (i, std::log (x.get (i)));

    return y;
}

/** SIMD implementation of std::log */
template <typename T>
inline juce::dsp::SIMDRegister<T> log10SIMD (juce::dsp::SIMDRegister<T> x)
{
    auto y = juce::dsp::SIMDRegister<T> ((T) 0);
    for (size_t i = 0; i < x.size(); ++i)
        y.set (i, std::log10 (x.get (i)));

    return y;
}

/** SIMD implementation of std::pow */
template <typename T>
inline juce::dsp::SIMDRegister<T> powSIMD (juce::dsp::SIMDRegister<T> a, juce::dsp::SIMDRegister<T> b)
{
    auto y = juce::dsp::SIMDRegister<T> ((T) 0);
    for (size_t i = 0; i < a.size(); ++i)
        y.set (i, std::pow (a.get (i), b.get (i)));

    return y;
}

/** SIMD implementation of std::sqrt */
template <typename T>
inline juce::dsp::SIMDRegister<T> sqrtSIMD (juce::dsp::SIMDRegister<T> x)
{
    auto y = juce::dsp::SIMDRegister<T> ((T) 0);
    for (size_t i = 0; i < x.size(); ++i)
        y.set (i, std::sqrt (x.get (i)));

    return y;
}

/** SIMD implementation of std::sin */
template <typename T>
inline juce::dsp::SIMDRegister<T> sinSIMD (juce::dsp::SIMDRegister<T> x)
{
    auto y = juce::dsp::SIMDRegister<T> ((T) 0);
    for (size_t i = 0; i < x.size(); ++i)
        y.set (i, std::sin (x.get (i)));

    return y;
}

/** SIMD implementation of std::cos */
template <typename T>
inline juce::dsp::SIMDRegister<T> cosSIMD (juce::dsp::SIMDRegister<T> x)
{
    auto y = juce::dsp::SIMDRegister<T> ((T) 0);
    for (size_t i = 0; i < x.size(); ++i)
        y.set (i, std::cos (x.get (i)));

    return y;
}

/** SIMD implementation of std::tan */
template <typename T>
inline juce::dsp::SIMDRegister<T> tanSIMD (juce::dsp::SIMDRegister<T> x)
{
    auto y = juce::dsp::SIMDRegister<T> ((T) 0);
    for (size_t i = 0; i < x.size(); ++i)
        y.set (i, std::tan (x.get (i)));

    return y;
}

/** SIMD implementation of std::sinh */
template <typename T>
inline juce::dsp::SIMDRegister<T> sinhSIMD (juce::dsp::SIMDRegister<T> x)
{
    auto y = juce::dsp::SIMDRegister<T> ((T) 0);
    for (size_t i = 0; i < x.size(); ++i)
        y.set (i, std::sinh (x.get (i)));

    return y;
}

/** SIMD implementation of std::cosh */
template <typename T>
inline juce::dsp::SIMDRegister<T> coshSIMD (juce::dsp::SIMDRegister<T> x)
{
    auto y = juce::dsp::SIMDRegister<T> ((T) 0);
    for (size_t i = 0; i < x.size(); ++i)
        y.set (i, std::cosh (x.get (i)));

    return y;
}

/** SIMD implementation of std::tanh */
template <typename T>
inline juce::dsp::SIMDRegister<T> tanhSIMD (juce::dsp::SIMDRegister<T> x)
{
    auto y = juce::dsp::SIMDRegister<T> ((T) 0);
    for (size_t i = 0; i < x.size(); ++i)
        y.set (i, std::tanh (x.get (i)));

    return y;
}

/** SIMD implementation of std::isnan */
template <typename T>
inline typename juce::dsp::SIMDRegister<T>::vMaskType isnanSIMD (juce::dsp::SIMDRegister<T> x)
{
    auto y = juce::dsp::SIMDRegister<T>();
    for (size_t i = 0; i < x.size(); ++i)
        y.set (i, (T) std::isnan (x.get (i)));

    using Vec = juce::dsp::SIMDRegister<T>;
    return Vec::notEqual (y, (Vec) 0);
}
#endif // CHOWDSP_USE_XSIMD

#ifdef __AVX2__
/** Default SIMG register alignment */
constexpr int CHOWDSP_DEFAULT_SIMD_ALIGNMENT = 32;
#else
/** Default SIMG register alignment */
constexpr int CHOWDSP_DEFAULT_SIMD_ALIGNMENT = 16;
#endif

/** Returns the maximum value from the SIMD register */
template <typename T>
inline T hMaxSIMD (juce::dsp::SIMDRegister<T> x)
{
    constexpr auto vecSize = juce::dsp::SIMDRegister<T>::size();
    T v alignas (CHOWDSP_DEFAULT_SIMD_ALIGNMENT)[vecSize];
    x.copyToRawArray (v);

    if constexpr (vecSize == 2)
        return juce::jmax (v[0], v[1]);
    else if constexpr (vecSize == 4)
        return juce::jmax (v[0], v[1], v[2], v[3]);
    else
        return juce::jmax (juce::jmax (v[0], v[1], v[2], v[3]), juce::jmax (v[4], v[5], v[6], v[7]));
}

/** Returns the minimum value from the SIMD register */
template <typename T>
inline T hMinSIMD (juce::dsp::SIMDRegister<T> x)
{
    constexpr auto vecSize = juce::dsp::SIMDRegister<T>::size();
    T v alignas (CHOWDSP_DEFAULT_SIMD_ALIGNMENT)[vecSize];
    x.copyToRawArray (v);

    if constexpr (vecSize == 2)
        return juce::jmin (v[0], v[1]);
    else if constexpr (vecSize == 4)
        return juce::jmin (v[0], v[1], v[2], v[3]);
    else
        return juce::jmin (juce::jmin (v[0], v[1], v[2], v[3]), juce::jmin (v[4], v[5], v[6], v[7]));
}

/** Returns the maximum absolute value from the SIMD register */
template <typename T>
inline T hAbsMaxSIMD (juce::dsp::SIMDRegister<T> x)
{
    return hMaxSIMD (juce::dsp::SIMDRegister<T>::abs (x));
}
} // namespace chowdsp::SIMDUtils
