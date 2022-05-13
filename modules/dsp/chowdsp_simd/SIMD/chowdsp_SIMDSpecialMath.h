#pragma once

namespace chowdsp::SIMDUtils
{
/**
 *  JUCE SIMD registers don't directly support more compilcated math functions
 *  We can use xsimd to enable these operations.
 */

template <typename T>
using x_type = xsimd::batch<T>;

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

/** SIMD implementation of std::atan2 */
template <typename T>
inline juce::dsp::SIMDRegister<T> atan2SIMD (juce::dsp::SIMDRegister<T> y, juce::dsp::SIMDRegister<T> x)
{
    return (juce::dsp::SIMDRegister<T>) xsimd::atan2 ((x_type<T>) y.value, (x_type<T>) x.value);
}

/** SIMD implementation of sincos */
template <typename T>
inline std::pair<juce::dsp::SIMDRegister<T>, juce::dsp::SIMDRegister<T>> sincosSIMD (juce::dsp::SIMDRegister<T> x)
{
    auto [sin, cos] = xsimd::sincos ((x_type<T>) x.value);
    return std::make_pair (juce::dsp::SIMDRegister<T> (sin), juce::dsp::SIMDRegister<T> (cos));
}

/** SIMD implementation of std::isnan */
template <typename T>
inline SampleTypeHelpers::vMaskTypeSIMD<T> isnanSIMD (juce::dsp::SIMDRegister<T> x)
{
    // For some reason, xsimd::isnan returns a batch of doubles when using SSE
    // but returns a batch of unsigned ints when using ARM NEON.
#if JUCE_ARM
    return (SampleTypeHelpers::vMaskTypeSIMD<T>) xsimd::isnan ((x_type<T>) x.value);
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

/** SIMD implementation of std::atan2 */
template <>
inline juce::dsp::SIMDRegister<double> atan2SIMD (juce::dsp::SIMDRegister<double> y, juce::dsp::SIMDRegister<double> x)
{
    auto res = juce::dsp::SIMDRegister<double> ((double) 0);
    for (size_t i = 0; i < x.size(); ++i)
        res.set (i, std::atan2 (y.get (i), x.get (i)));

    return res;
}

template <>
inline std::pair<juce::dsp::SIMDRegister<double>, juce::dsp::SIMDRegister<double>> sincosSIMD (juce::dsp::SIMDRegister<double> x)
{
    auto sin = juce::dsp::SIMDRegister<double> ((double) 0);
    auto cos = juce::dsp::SIMDRegister<double> ((double) 0);
    for (size_t i = 0; i < x.size(); ++i)
    {
        sin.set (i, std::sin (x.get (i)));
        cos.set (i, std::cos (x.get (i)));
    }

    return std::make_pair (sin, cos);
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

/** Returns the maximum value from the SIMD register */
template <typename T>
[[deprecated]] inline T hMaxSIMD (juce::dsp::SIMDRegister<T> x)
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

/** Returns the maximum value from the SIMD register */
template <typename T>
inline T hMaxSIMD (const xsimd::batch<T>& x)
{
    constexpr auto vecSize = xsimd::batch<T>::size;
    T v alignas (CHOWDSP_DEFAULT_SIMD_ALIGNMENT)[vecSize];
    xsimd::store_aligned (v, x);

    if constexpr (vecSize == 2)
        return juce::jmax (v[0], v[1]);
    else if constexpr (vecSize == 4)
        return juce::jmax (v[0], v[1], v[2], v[3]);
    else
        return juce::jmax (juce::jmax (v[0], v[1], v[2], v[3]), juce::jmax (v[4], v[5], v[6], v[7]));
}

/** Returns the minimum value from the SIMD register */
template <typename T>
[[deprecated]] inline T hMinSIMD (juce::dsp::SIMDRegister<T> x)
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

/** Returns the minimum value from the SIMD register */
template <typename T>
inline T hMinSIMD (const xsimd::batch<T>& x)
{
    constexpr auto vecSize = xsimd::batch<T>::size;
    T v alignas (CHOWDSP_DEFAULT_SIMD_ALIGNMENT)[vecSize];
    xsimd::store_aligned (v, x);

    if constexpr (vecSize == 2)
        return juce::jmin (v[0], v[1]);
    else if constexpr (vecSize == 4)
        return juce::jmin (v[0], v[1], v[2], v[3]);
    else
        return juce::jmin (juce::jmin (v[0], v[1], v[2], v[3]), juce::jmin (v[4], v[5], v[6], v[7]));
}

/** Returns the maximum absolute value from the SIMD register */
template <typename T>
[[deprecated]] inline T hAbsMaxSIMD (juce::dsp::SIMDRegister<T> x)
{
    return hMaxSIMD (juce::dsp::SIMDRegister<T>::abs (x));
}

/** Returns the maximum absolute value from the SIMD register */
template <typename T>
inline T hAbsMaxSIMD (const xsimd::batch<T>& x)
{
    return hMaxSIMD (xsimd::abs (x));
}
} // namespace chowdsp::SIMDUtils
