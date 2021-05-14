#pragma once

namespace chowdsp
{
namespace SIMDUtils
{
    /**
JUCE doesn't implement the divide operator for
SIMDRegister, so here's a simple implementation.

Based on: https://forum.juce.com/t/divide-by-simdregister/28968/18
*/
    using vec4 = juce::dsp::SIMDRegister<float>;
    using vec2 = juce::dsp::SIMDRegister<double>;

#if defined(__i386__) || defined(__amd64__) || defined(_M_X64) || defined(_X86_) || defined(_M_IX86)
    inline vec4 operator/ (const vec4& l, const vec4& r)
    {
        return _mm_div_ps (l.value, r.value);
    }

    inline vec2 operator/ (const vec2& l, const vec2& r)
    {
        return _mm_div_pd (l.value, r.value);
    }

#elif defined(_M_ARM64) || defined(__arm64__) || defined(__aarch64__)
    inline vec4 operator/ (const vec4& l, const vec4& r)
    {
        return vdivq_f32 (l.value, r.value);
    }

    inline vec2 operator/ (const vec2& l, const vec2& r)
    {
        // @TODO: figure out how to override JUCE ARM NEON code
        // return vdivq_f64 (l.value, r.value);
        return { { l.value.v[0] / r.value.v[0], l.value.v[1] / r.value.v[1] } };
    }

#else
    JUCE_COMPILER_WARNING ("SIMD divide not implemented for this platform... using non-vectorized implementation")

    inline vec4 operator/ (const vec4& l, const vec4& r)
    {
        vec4 out (l);
        out[0] = out[0] / r[0];
        out[1] = out[1] / r[1];
        out[2] = out[2] / r[2];
        out[3] = out[3] / r[3];
        return out;
    }

    inline vec2 operator/ (const vec2& l, const vec2& r)
    {
        vec2 out (l);
        out[0] = out[0] / r[0];
        out[1] = out[1] / r[1];
        return out;
    }
#endif

    //============================================================
    /** JUCE doesn't natively support loading unaligned SIMD registers,
 *  but most SIMD instruction sets do have load unaligned instructions,
 *  so let's implement them where we can!
 */
    static inline vec4 loadUnaligned (const float* ptr)
    {
#if defined(__i386__) || defined(__amd64__) || defined(_M_X64) || defined(_X86_) || defined(_M_IX86)
        return vec4 (_mm_loadu_ps (ptr));
#elif defined(_M_ARM64) || defined(__arm64__) || defined(__aarch64__)
        return vec4 (vld1q_f32 (ptr));
#else
        // fallback implementation
        auto reg = vec4 (0.0f);
        auto* regPtr = reinterpret_cast<float*> (&reg.value);
        std::copy (ptr, ptr + vec4::size(), regPtr);
        return reg;
#endif
    }

    static inline vec2 loadUnaligned (const double* ptr)
    {
#if defined(__i386__) || defined(__amd64__) || defined(_M_X64) || defined(_X86_) || defined(_M_IX86)
        return vec2 (_mm_loadu_pd (ptr));
#elif defined(_M_ARM64) || defined(__arm64__) || defined(__aarch64__)
        // @TODO: figure out how to override JUCE ARM NEON code
        // return vec2 (vld1q_f64 (ptr));
        auto reg = vec2 (0.0);
        auto* regPtr = reinterpret_cast<double*> (&reg.value);
        std::copy (ptr, ptr + vec2::size(), regPtr);
        return reg;
#else
        // fallback implementation
        auto reg = vec2 (0.0);
        auto* regPtr = reinterpret_cast<double*> (&reg.value);
        std::copy (ptr, ptr + vec2::size(), regPtr);
        return reg;
#endif
    }

    //============================================================
    /**
 * Fast implementation of the JUCE
 * FastMathapproximations::sin function
 * with SIMD optomisation.
 */
    template <typename T>
    inline juce::dsp::SIMDRegister<T> fastsinSIMD (juce::dsp::SIMDRegister<T> x) noexcept
    {
        const auto m11511339840 = juce::dsp::SIMDRegister<T> ((T) 11511339840);
        const auto m1640635920 = juce::dsp::SIMDRegister<T> ((T) 1640635920);
        const auto m52785432 = juce::dsp::SIMDRegister<T> ((T) 52785432);
        const auto m479249 = juce::dsp::SIMDRegister<T> ((T) 479249);
        const auto m277920720 = juce::dsp::SIMDRegister<T> ((T) 277920720);
        const auto m3177720 = juce::dsp::SIMDRegister<T> ((T) 3177720);
        const auto m18361 = juce::dsp::SIMDRegister<T> ((T) 18361);
        const auto mnegone = juce::dsp::SIMDRegister<T> ((T) -1);

        auto x2 = x * x;
        auto num = mnegone * (x * ((x2 * (m1640635920 + (x2 * ((x2 * m479249) - m52785432)))) - m11511339840));
        auto den = m11511339840 + (x2 * (m277920720 + (x2 * (m3177720 + (x2 * m18361)))));

        return num / den;
    }

    /**
 * Fast implementation of the JUCE
 * FastMathapproximations::cos function
 * with SIMD optomisation.
 */
    template <typename T>
    inline juce::dsp::SIMDRegister<T> fastcosSIMD (juce::dsp::SIMDRegister<T> x) noexcept
    {
        const auto m39251520 = juce::dsp::SIMDRegister<T> ((T) 39251520);
        const auto m18471600 = juce::dsp::SIMDRegister<T> ((T) 18471600);
        const auto m1075032 = juce::dsp::SIMDRegister<T> ((T) 1075032);
        const auto m14615 = juce::dsp::SIMDRegister<T> ((T) 14615);
        const auto m1154160 = juce::dsp::SIMDRegister<T> ((T) 1154160);
        const auto m16632 = juce::dsp::SIMDRegister<T> ((T) 16632);
        const auto m127 = juce::dsp::SIMDRegister<T> ((T) 127);

        auto x2 = x * x;
        auto num = m39251520 - (x2 * (m18471600 + (x2 * ((m14615 * x2) - m1075032))));
        auto den = m39251520 + (x2 * (m1154160 + (x2 * (m16632 + (x2 * m127)))));

        return num / den;
    }

    /** Clamps the input value to the range (-pi, pi) */
    template <typename T>
    inline juce::dsp::SIMDRegister<T> clampToPiRangeSIMD (juce::dsp::SIMDRegister<T> x)
    {
        const auto mpi = juce::dsp::SIMDRegister<T> (juce::MathConstants<T>::pi);
        const auto m2pi = juce::dsp::SIMDRegister<T> (juce::MathConstants<T>::twoPi);
        const auto oo2p = juce::dsp::SIMDRegister<T> ((T) 1.0 / juce::MathConstants<T>::twoPi);
        const auto mz = juce::dsp::SIMDRegister<T> ((T) 0);

        auto y = x + mpi;
        auto yip = juce::dsp::SIMDRegister<T>::truncate (y * oo2p);
        auto p = y - (m2pi * yip);
        auto off = m2pi & juce::dsp::SIMDRegister<T>::lessThan (p, mz);
        p = p + off;
        return p - mpi;
    }

    //============================================================
    /** JUCE SIMD registers don't directly support complex math functions
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

#if defined(_M_ARM64) || defined(__arm64__) || defined(__aarch64__)
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
#endif // defined(_M_ARM64) || defined(__arm64__) || defined(__aarch64__)

#else // fallback implemetations
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
#endif

} // namespace SIMDUtils

} // namespace chowdsp