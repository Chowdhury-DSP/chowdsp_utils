#pragma once

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsign-conversion",
                                     "-Waggressive-loop-optimizations")

/** Useful methods for working with and evaluating polynomials */
namespace chowdsp::Polynomials
{
/** Useful template type representing the product of two other types. */
template <typename T, typename X>
using P = decltype (T {} * X {});

/**
 * Evaluates a polynomial of a given order, using a naive method.
 * Coefficients should be given in the form { a_n, a_n-1, ..., a_1, a_0 }
 */
template <int ORDER, typename T, typename X>
constexpr P<T, X> naive (const T (&coeffs)[ORDER + 1], X x)
{
    CHOWDSP_USING_XSIMD_STD (pow);

    P<T, X> sum = coeffs[ORDER];
    for (int n = 0; n < ORDER; ++n)
        sum += coeffs[n] * pow (x, X (ORDER - n));

    return sum;
}

/**
 * Evaluates a polynomial of a given order, using Horner's method.
 * Coefficients should be given in the form { a_n, a_n-1, ..., a_1, a_0 }
 * https://en.wikipedia.org/wiki/Horner%27s_method
 */
template <int ORDER, typename T, typename X>
constexpr P<T, X> horner (const T (&coeffs)[ORDER + 1], X x)
{
    P<T, X> b = coeffs[0];
    for (int n = 1; n <= ORDER; ++n)
        b = b * x + coeffs[n];

    return b;
}

/**
 * Evaluates a polynomial of a given order, using Estrin's scheme.
 * Coefficients should be given in the form { a_n, a_n-1, ..., a_1, a_0 }
 * https://en.wikipedia.org/wiki/Estrin%27s_scheme
 */
template <int ORDER, typename T, typename X>
constexpr P<T, X> estrin (const T (&coeffs)[ORDER + 1], X x)
{
    if constexpr (ORDER <= 1) // base case
    {
        return coeffs[1] + coeffs[0] * x;
    }
    else
    {
        P<T, X> temp[ORDER / 2 + 1];
        for (int n = ORDER; n > 0; n -= 2)
            temp[n / 2] = coeffs[n] + coeffs[n - 1] * x;

        if constexpr (ORDER % 2 == 0) // even order polynomial
            temp[0] = coeffs[0];

        return estrin<ORDER / 2> (temp, x * x); // recurse!
    }
}

/**
 * Computes the coefficients of the antiderivative of a polynomial.
 * Coefficients should be given in the form { a_n, a_n-1, ..., a_1, a_0 }
 */
template <int ORDER, typename T>
constexpr void antiderivative (const T (&coeffs)[ORDER + 1], T (&ad_coeffs)[ORDER + 2], T C = (T) 0)
{
    for (int n = 0; n <= ORDER; ++n)
        ad_coeffs[n] = coeffs[n] / T (ORDER + 1 - n);

    ad_coeffs[ORDER + 1] = C;
}

/**
 * Computes the coefficients of the derivative of a polynomial.
 * Coefficients should be given in the form { a_n, a_n-1, ..., a_1, a_0 }
 */
template <int ORDER, typename T>
constexpr void derivative (const T (&coeffs)[ORDER + 1], T (&d_coeffs)[ORDER])
{
    for (int n = 0; n < ORDER; ++n)
        d_coeffs[n] = coeffs[n] * T (ORDER - n);
}
} // namespace chowdsp::Polynomials

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
