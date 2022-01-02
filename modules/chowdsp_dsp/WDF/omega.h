/*
 * Copyright (C) 2019 Stefano D'Angelo
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * NOTE: code has been modified significantly by Jatin Chowdhury
 */

#ifndef OMEGA_H_INCLUDED
#define OMEGA_H_INCLUDED

#include <algorithm>

/**
 * Useful approximations for evaluating the Wright Omega function.
 *
 * This approach was devloped by Stefano D'Angelo, and adapted from his
 * original implementation under the MIT license.
 * - Paper: https://www.dafx.de/paper-archive/2019/DAFx2019_paper_5.pdf
 * - Original Source: https://www.dangelo.audio/dafx2019-omega.html
 */
namespace chowdsp::Omega
{
/** approximation for log_2(x), optimized on the range [1, 2] */
template <typename T>
inline T log2_approx (T x)
{
    constexpr T alpha = (T) 0.1640425613334452;
    constexpr T beta = (T) -1.098865286222744;
    constexpr T gamma = (T) 3.148297929334117;
    constexpr T zeta = (T) -2.213475204444817;

    return zeta + x * (gamma + x * (beta + x * alpha));
}

/** approximation for log(x) */
template <typename T>
T log_approx (T x);

/** approximation for log(x) (32-bit) */
template <>
[[maybe_unused]] inline float log_approx (float x)
{
    union
    {
        int32_t i;
        float f;
    } v {};
    v.f = x;
    int32_t ex = v.i & 0x7f800000;
    int32_t e = (ex >> 23) - 127;
    v.i = (v.i - ex) | 0x3f800000;

    return 0.693147180559945f * ((float) e + log2_approx<float> (v.f));
}

/** approximation for log(x) (64-bit) */
template <>
[[maybe_unused]] inline double log_approx (double x)
{
    union
    {
        int64_t i;
        double d;
    } v {};
    v.d = x;
    int64_t ex = v.i & 0x7ff0000000000000;
    int64_t e = (ex >> 53) - 510;
    v.i = (v.i - ex) | 0x3ff0000000000000;

    return 0.693147180559945 * ((double) e + log2_approx<double> (v.d));
}

/** approximation for 2^x, optimized on the range [0, 1] */
template <typename T>
inline T pow2_approx (T x)
{
    constexpr T alpha = (T) 0.07944154167983575;
    constexpr T beta = (T) 0.2274112777602189;
    constexpr T gamma = (T) 0.6931471805599453;
    constexpr T zeta = (T) 1.0;

    return zeta + x * (gamma + x * (beta + x * alpha));
}

/** approximation for exp(x) */
template <typename T>
T exp_approx (T x);

/** approximation for exp(x) (32-bit) */
template <>
[[maybe_unused]] inline float exp_approx (float x)
{
    x = std::max (-126.0f, 1.442695040888963f * x);

    union
    {
        int32_t i;
        float f;
    } v {};

    auto xi = (int32_t) x;
    int32_t l = x < 0.0f ? xi - 1 : xi;
    float f = x - (float) l;
    v.i = (l + 127) << 23;

    return v.f * pow2_approx<float> (f);
}

/** approximation for exp(x) (64-bit) */
template <>
[[maybe_unused]] inline double exp_approx (double x)
{
    x = std::max (-126.0, 1.442695040888963 * x);

    union
    {
        int64_t i;
        double d;
    } v {};

    auto xi = (int64_t) x;
    int64_t l = x < 0.0 ? xi - 1 : xi;
    double d = x - (double) l;
    v.i = (l + 1023) << 52;

    return v.d * pow2_approx<double> (d);
}

/** First-order approximation of the Wright Omega functions */
template <typename T>
inline T omega1 (T x)
{
    return std::max (x, (T) 0);
}

/** Second-order approximation of the Wright Omega functions */
template <typename T>
inline T omega2 (T x)
{
    const T x1 = (T) -3.684303659906469;
    const T x2 = (T) 1.972967391708859;
    const T a = (T) 9.451797158780131e-3;
    const T b = (T) 1.126446405111627e-1;
    const T c = (T) 4.451353886588814e-1;
    const T d = (T) 5.836596684310648e-1;
    return x < x1 ? 0.f : (x > x2 ? x : d + x * (c + x * (b + x * a)));
}

/** Third-order approximation of the Wright Omega functions */
template <typename T>
inline T omega3 (T x)
{
    constexpr T x1 = (T) -3.341459552768620;
    constexpr T x2 = (T) 8.0;
    constexpr T a = (T) -1.314293149877800e-3;
    constexpr T b = (T) 4.775931364975583e-2;
    constexpr T c = (T) 3.631952663804445e-1;
    constexpr T d = (T) 6.313183464296682e-1;
    return x < x1 ? 0.f : (x < x2 ? d + x * (c + x * (b + x * a)) : x - log_approx<T> (x));
}

/** Fourth-order approximation of the Wright Omega functions */
template <typename T>
inline T omega4 (T x)
{
    const T y = omega3<T> (x);
    return y - (y - exp_approx<T> (x - y)) / (y + (T) 1);
}

#if WDF_USING_JUCE
/** First-order approximation of the Wright Omega functions */
template <typename T>
inline juce::dsp::SIMDRegister<T> omega1 (juce::dsp::SIMDRegister<T> x)
{
    return juce::dsp::SIMDRegister<T>::max (x, juce::dsp::SIMDRegister<T> (0));
}

/** Second-order approximation of the Wright Omega functions */
template <typename T>
inline juce::dsp::SIMDRegister<T> omega2 (juce::dsp::SIMDRegister<T> x)
{
    auto y = juce::dsp::SIMDRegister<T> ((T) 0);
    for (size_t i = 0; i < x.size(); ++i)
        y.set (i, omega2 (x.get (i)));
    return y;
}

/** Third-order approximation of the Wright Omega functions */
template <typename T>
inline juce::dsp::SIMDRegister<T> omega3 (juce::dsp::SIMDRegister<T> x)
{
    auto y = juce::dsp::SIMDRegister<T> ((T) 0);
    for (size_t i = 0; i < x.size(); ++i)
        y.set (i, omega3 (x.get (i)));
    return y;
}

/** Fourth-order approximation of the Wright Omega functions */
template <typename T>
inline juce::dsp::SIMDRegister<T> omega4 (juce::dsp::SIMDRegister<T> x)
{
    auto y = juce::dsp::SIMDRegister<T> ((T) 0);
    for (size_t i = 0; i < x.size(); ++i)
        y.set (i, omega4 (x.get (i)));
    return y;
}
#endif // WDF_USING_JUCE

} // namespace chowdsp::Omega

#endif //OMEGA_H_INCLUDED
