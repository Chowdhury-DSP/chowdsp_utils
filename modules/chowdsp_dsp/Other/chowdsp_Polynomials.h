#pragma once

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsign-conversion",
                                     "-Waggressive-loop-optimizations")

namespace chowdsp
{
/** Useful methods for working with and evaluating polynomials */
namespace Polynomials
{
    /**
     * Evaluates a polynomial of a given order, using a naive method.
     * Coefficients should be given in the form { a_n, a_n-1, ..., a_1, a_0 }
     */
    template <int ORDER, typename T>
    inline T naive (const T (&coeffs)[ORDER + 1], const T x)
    {
        T sum = (T) 0;
        for (int n = 0; n <= ORDER; ++n)
            sum += coeffs[n] * std::pow (x, T (ORDER - n));

        return sum;
    }

    /**
     * Evaluates a polynomial of a given order, using Horner's method.
     * Coefficients should be given in the form { a_n, a_n-1, ..., a_1, a_0 }
     * https://en.wikipedia.org/wiki/Horner%27s_method
     */
    template <int ORDER, typename T>
    inline T horner (const T (&coeffs)[ORDER + 1], const T x)
    {
        T b = coeffs[0];
        for (int n = 1; n <= ORDER; ++n)
            b = b * x + coeffs[n];

        return b;
    }

    /**
     * Evaluates a polynomial of a given order, using Estrin's scheme.
     * Coefficients should be given in the form { a_n, a_n-1, ..., a_1, a_0 }
     * https://en.wikipedia.org/wiki/Estrin%27s_scheme
     */
    template <int ORDER, typename T>
    inline T estrin (const T (&coeffs)[ORDER + 1], const T x)
    {
        if constexpr (ORDER <= 1) // base case
        {
            return coeffs[1] + coeffs[0] * x;
        }
        else
        {
            T temp[ORDER / 2 + 1];
            for (int n = ORDER; n >= 0; n -= 2)
                temp[n / 2] = coeffs[n] + coeffs[n - 1] * x;

            if constexpr (ORDER % 2 == 0) // even order polynomial
                temp[0] = coeffs[0];

            return estrin<ORDER / 2> (temp, x * x); // recurse!
        }
    }
} // namespace Polynomials
} // namespace chowdsp

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
