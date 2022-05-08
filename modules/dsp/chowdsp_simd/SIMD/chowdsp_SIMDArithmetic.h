#pragma once

namespace chowdsp::SIMDUtils
{
/**
JUCE doesn't implement the divide operator for
SIMDRegister, so here's a simple implementation.

Based on: https://forum.juce.com/t/divide-by-simdregister/28968/18
*/

#if defined(__i386__) || defined(__amd64__) || defined(_M_X64) || defined(_X86_) || defined(_M_IX86)
/** Divides another SIMDRegister to the receiver. */
inline vec4 operator/ (const vec4& l, const vec4& r)
{
#ifdef __AVX2__
    return _mm256_div_ps (l.value, r.value);
#else
    return _mm_div_ps (l.value, r.value);
#endif
}

/** Divides another SIMDRegister to the receiver. */
inline vec2 operator/ (const vec2& l, const vec2& r)
{
#ifdef __AVX2__
    return _mm256_div_pd (l.value, r.value);
#else
    return _mm_div_pd (l.value, r.value);
#endif
}

#elif defined(_M_ARM64) || defined(__arm64__) || defined(__aarch64__)
/** Divides another SIMDRegister to the receiver. */
inline vec4 operator/ (const vec4& l, const vec4& r)
{
    return vdivq_f32 (l.value, r.value);
}

/** Divides another SIMDRegister to the receiver. */
inline vec2 operator/ (const vec2& l, const vec2& r)
{
#if CHOWDSP_USE_CUSTOM_JUCE_DSP
    return vdivq_f64 (l.value, r.value);
#else
    return { { l.value.v[0] / r.value.v[0], l.value.v[1] / r.value.v[1] } };
#endif
}

#else
JUCE_COMPILER_WARNING ("SIMD divide not implemented for this platform... using non-vectorized implementation")

/** Divides another SIMDRegister to the receiver. */
inline vec4 operator/ (const vec4& l, const vec4& r)
{
    vec4 out (l);
    out[0] = out[0] / r[0];
    out[1] = out[1] / r[1];
    out[2] = out[2] / r[2];
    out[3] = out[3] / r[3];
    return out;
}

/** Divides another SIMDRegister to the receiver. */
inline vec2 operator/ (const vec2& l, const vec2& r)
{
    vec2 out (l);
    out[0] = out[0] / r[0];
    out[1] = out[1] / r[1];
    return out;
}
#endif

//============================================================
/**
 * JUCE SIMD has the correct overloads for dsp::SIMDRegister<T> * T,
 * but not for T * dsp::SIMDRegister<T>. So let's implement them here...
 */

/** Computes the sum of a scalar and SIMDRegister */
template <typename T>
inline juce::dsp::SIMDRegister<T> operator+ (T l, const juce::dsp::SIMDRegister<T>& r)
{
    return r + l;
}

/** Computes the difference of a scalar and SIMDRegister */
template <typename T>
inline juce::dsp::SIMDRegister<T> operator- (T l, const juce::dsp::SIMDRegister<T>& r)
{
    return (juce::dsp::SIMDRegister<T>) l - r;
}

/** Computes the product of a scalar and SIMDRegister */
template <typename T>
inline juce::dsp::SIMDRegister<T> operator* (T l, const juce::dsp::SIMDRegister<T>& r)
{
    return r * l;
}

/** Computes the quotient of a scalar and SIMDRegister */
template <typename T>
inline juce::dsp::SIMDRegister<T> operator/ (T l, const juce::dsp::SIMDRegister<T>& r)
{
    return (juce::dsp::SIMDRegister<T>) l / r;
}

/** Computes the unary minus of a SIMDRegister */
template <typename T>
inline juce::dsp::SIMDRegister<T> operator- (const juce::dsp::SIMDRegister<T>& x)
{
    return juce::dsp::SIMDRegister<T>() - x;
}
} // namespace chowdsp::SIMDUtils
