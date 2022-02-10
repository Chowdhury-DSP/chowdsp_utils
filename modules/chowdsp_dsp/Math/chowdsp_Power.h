#pragma once

#include <type_traits>

/** Useful methods for computing the power function for integer exponents in the range [0, 16] */
namespace chowdsp::Power
{
// Template specialization of exponent methods using
// Addition-chain exponentiation (https://en.wikipedia.org/wiki/Addition-chain_exponentiation).

/** Optimized integer power method. */
template <int exp, typename T>
constexpr typename std::enable_if<exp == 0, T>::type
    ipow (T) noexcept
{
    return (T) 1;
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 0, void>::type
    ipow (T* dest, const T*, int numSamples) noexcept
{
    juce::FloatVectorOperations::fill (dest, (T) 1, numSamples);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 1, T>::type
    ipow (T a) noexcept
{
    return a;
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 1, void>::type
    ipow (T* dest, const T* a, int numSamples) noexcept
{
    juce::FloatVectorOperations::copy (dest, a, numSamples);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 2, T>::type
    ipow (T a) noexcept
{
    return a * a;
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 2, void>::type
    ipow (T* dest, const T* a, int numSamples) noexcept
{
    juce::FloatVectorOperations::multiply (dest, a, a, numSamples);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 3, T>::type
    ipow (T a) noexcept
{
    return ipow<2> (a) * a;
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 3, void>::type
    ipow (T* dest, const T* a, int numSamples) noexcept
{
    T* b = (T*) alloca ((size_t) numSamples * sizeof (T));
    ipow<2> (b, a, numSamples);
    juce::FloatVectorOperations::multiply (dest, b, a, numSamples);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 4, T>::type
    ipow (T a) noexcept
{
    const auto b = ipow<2> (a);
    return ipow<2> (b);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 4, void>::type
    ipow (T* dest, const T* a, int numSamples) noexcept
{
    T* b = (T*) alloca ((size_t) numSamples * sizeof (T));
    ipow<2> (b, a, numSamples);
    ipow<2> (dest, b, numSamples);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 5, T>::type
    ipow (T a) noexcept
{
    return ipow<4> (a) * a;
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 5, void>::type
    ipow (T* dest, const T* a, int numSamples) noexcept
{
    T* b = (T*) alloca ((size_t) numSamples * sizeof (T));
    ipow<4> (b, a, numSamples);
    juce::FloatVectorOperations::multiply (dest, b, a, numSamples);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 6, T>::type
    ipow (T a) noexcept
{
    const auto b = ipow<2> (a);
    return ipow<3> (b);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 6, void>::type
    ipow (T* dest, const T* a, int numSamples) noexcept
{
    T* b = (T*) alloca ((size_t) numSamples * sizeof (T));
    ipow<2> (b, a, numSamples);
    ipow<3> (dest, b, numSamples);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 7, T>::type
    ipow (T a) noexcept
{
    return ipow<6> (a) * a;
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 7, void>::type
    ipow (T* dest, const T* a, int numSamples) noexcept
{
    T* b = (T*) alloca ((size_t) numSamples * sizeof (T));
    ipow<6> (b, a, numSamples);
    juce::FloatVectorOperations::multiply (dest, b, a, numSamples);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 8, T>::type
    ipow (T a) noexcept
{
    const auto d = ipow<4> (a);
    return ipow<2> (d);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 8, void>::type
    ipow (T* dest, const T* a, int numSamples) noexcept
{
    T* d = (T*) alloca ((size_t) numSamples * sizeof (T));
    ipow<4> (d, a, numSamples);
    ipow<2> (dest, d, numSamples);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 9, T>::type
    ipow (T a) noexcept
{
    const auto c = ipow<3> (a);
    return ipow<3> (c);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 9, void>::type
    ipow (T* dest, const T* a, int numSamples) noexcept
{
    T* c = (T*) alloca ((size_t) numSamples * sizeof (T));
    ipow<3> (c, a, numSamples);
    ipow<3> (dest, c, numSamples);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 10, T>::type
    ipow (T a) noexcept
{
    const auto e = ipow<5> (a);
    return ipow<2> (e);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 10, void>::type
    ipow (T* dest, const T* a, int numSamples) noexcept
{
    T* e = (T*) alloca ((size_t) numSamples * sizeof (T));
    ipow<5> (e, a, numSamples);
    ipow<2> (dest, e, numSamples);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 11, T>::type
    ipow (T a) noexcept
{
    return ipow<10> (a) * a;
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 11, void>::type
    ipow (T* dest, const T* a, int numSamples) noexcept
{
    T* b = (T*) alloca ((size_t) numSamples * sizeof (T));
    ipow<10> (b, a, numSamples);
    juce::FloatVectorOperations::multiply (dest, b, a, numSamples);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 12, T>::type
    ipow (T a) noexcept
{
    const auto d = ipow<4> (a);
    return ipow<3> (d);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 12, void>::type
    ipow (T* dest, const T* a, int numSamples) noexcept
{
    T* d = (T*) alloca ((size_t) numSamples * sizeof (T));
    ipow<4> (d, a, numSamples);
    ipow<3> (dest, d, numSamples);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 13, T>::type
    ipow (T a) noexcept
{
    return ipow<12> (a) * a;
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 13, void>::type
    ipow (T* dest, const T* a, int numSamples) noexcept
{
    T* b = (T*) alloca ((size_t) numSamples * sizeof (T));
    ipow<12> (b, a, numSamples);
    juce::FloatVectorOperations::multiply (dest, b, a, numSamples);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 14, T>::type
    ipow (T a) noexcept
{
    const auto g = ipow<7> (a);
    return ipow<2> (g);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 14, void>::type
    ipow (T* dest, const T* a, int numSamples) noexcept
{
    T* g = (T*) alloca ((size_t) numSamples * sizeof (T));
    ipow<7> (g, a, numSamples);
    ipow<2> (dest, g, numSamples);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 15, T>::type
    ipow (T a) noexcept
{
    const auto e = ipow<5> (a);
    return ipow<3> (e);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 15, void>::type
    ipow (T* dest, const T* a, int numSamples) noexcept
{
    T* e = (T*) alloca ((size_t) numSamples * sizeof (T));
    ipow<5> (e, a, numSamples);
    ipow<3> (dest, e, numSamples);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 16, T>::type
    ipow (T a) noexcept
{
    const auto h = ipow<8> (a);
    return ipow<2> (h);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 16, void>::type
    ipow (T* dest, const T* a, int numSamples) noexcept
{
    T* h = (T*) alloca ((size_t) numSamples * sizeof (T));
    ipow<8> (h, a, numSamples);
    ipow<2> (dest, h, numSamples);
}
} // namespace chowdsp::Power
