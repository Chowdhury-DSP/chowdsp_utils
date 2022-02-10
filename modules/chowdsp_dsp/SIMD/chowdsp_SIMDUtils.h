#pragma once

/** Useful extensions on juce::dsp::SIMDRegister */
namespace chowdsp::SIMDUtils
{
using vec4 = juce::dsp::SIMDRegister<float>;
using vec2 = juce::dsp::SIMDRegister<double>;

#ifdef __AVX2__
/** Default SIMG register alignment */
constexpr int CHOWDSP_DEFAULT_SIMD_ALIGNMENT = 32;
#else
/** Default SIMG register alignment */
constexpr int CHOWDSP_DEFAULT_SIMD_ALIGNMENT = 16;
#endif

//============================================================
/**
 *  JUCE doesn't natively support loading unaligned SIMD registers,
 *  but most SIMD instruction sets do have load unaligned instructions,
 *  so let's implement them where we can!
 */
inline vec4 loadUnaligned (const float* ptr)
{
#if defined(__i386__) || defined(__amd64__) || defined(_M_X64) || defined(_X86_) || defined(_M_IX86)
#ifdef __AVX2__
    return { _mm256_loadu_ps (ptr) };
#else
    return { _mm_loadu_ps (ptr) };
#endif

#elif defined(_M_ARM64) || defined(__arm64__) || defined(__aarch64__)
    return { vld1q_f32 (ptr) };
#else
    // fallback implementation
    auto reg = vec4 (0.0f);
    auto* regPtr = reinterpret_cast<float*> (&reg.value);
    std::copy (ptr, ptr + vec4::size(), regPtr);
    return reg;
#endif
}

inline vec2 loadUnaligned (const double* ptr)
{
#if defined(__i386__) || defined(__amd64__) || defined(_M_X64) || defined(_X86_) || defined(_M_IX86)
#ifdef __AVX2__
    return { _mm256_loadu_pd (ptr) };
#else
    return { _mm_loadu_pd (ptr) };
#endif

#elif defined(_M_ARM64) || defined(__arm64__) || defined(__aarch64__)
#if CHOWDSP_USE_CUSTOM_JUCE_DSP
    return { vld1q_f64 (ptr) };
#else
    auto reg = vec2 (0.0);
    auto* regPtr = reinterpret_cast<double*> (&reg.value);
    std::copy (ptr, ptr + vec2::size(), regPtr);
    return reg;
#endif
#else
    // fallback implementation
    auto reg = vec2 (0.0);
    auto* regPtr = reinterpret_cast<double*> (&reg.value);
    std::copy (ptr, ptr + vec2::size(), regPtr);
    return reg;
#endif
}

inline void storeUnaligned (float* ptr, const vec4& vec)
{
#if defined(__i386__) || defined(__amd64__) || defined(_M_X64) || defined(_X86_) || defined(_M_IX86)
#ifdef __AVX2__
    _mm256_storeu_ps (ptr, vec.value);
#else
    _mm_storeu_ps (ptr, vec.value);
#endif

#elif defined(_M_ARM64) || defined(__arm64__) || defined(__aarch64__)
    vst1q_f32 (ptr, vec.value);
#else
    // fallback implementation
    auto* regPtr = reinterpret_cast<float*> (&vec.value);
    std::copy (regPtr, regPtr + vec4::size(), ptr);
#endif
}

inline void storeUnaligned (double* ptr, const vec2& vec)
{
#if defined(__i386__) || defined(__amd64__) || defined(_M_X64) || defined(_X86_) || defined(_M_IX86)
#ifdef __AVX2__
    _mm256_storeu_pd (ptr, vec.value);
#else
    _mm_storeu_pd (ptr, vec.value);
#endif

#elif defined(_M_ARM64) || defined(__arm64__) || defined(__aarch64__)
#if CHOWDSP_USE_CUSTOM_JUCE_DSP
    vst1q_f64 (ptr, vec.value);
#else
    auto* regPtr = reinterpret_cast<double*> (&vec.value);
    std::copy (regPtr, regPtr + vec2::size(), ptr);
#endif
#else
    // fallback implementation
    auto* regPtr = reinterpret_cast<double*> (&vec.value);
    std::copy (regPtr, regPtr + vec2::size(), ptr);
#endif
}
} // namespace chowdsp::SIMDUtils
