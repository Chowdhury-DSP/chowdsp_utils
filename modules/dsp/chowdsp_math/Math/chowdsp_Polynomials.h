#pragma once

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsign-conversion",
                                     "-Waggressive-loop-optimizations")

namespace chowdsp
{
/**
 * Type tag to denote a polynomial stored with coefficients in descending order,
 * i.e. { a_n, a_n-1, ..., a_1, a_0 }.
 */
struct poly_order_descending
{
};

/**
 * Type tag to denote a polynomial stored with coefficients in ascending order,
 * i.e. { a_0, a_1, ..., a_n-1, a_n }.
 */
struct poly_order_ascending
{
};

/** An array wrapper for polynomial coefficients, stored in either ascending or descending order. */
template <typename T, size_t poly_order, typename storage_mode = poly_order_descending>
struct Polynomial
{
    static constexpr auto num_coeffs = poly_order + 1;
    std::array<T, num_coeffs> coeffs {};

    Polynomial() = default;

    constexpr Polynomial (const std::array<T, num_coeffs>& init_coeffs, [[maybe_unused]] storage_mode tag = storage_mode {}) // NOLINT(*-explicit-constructor)
    {
        set_coefficients (init_coeffs);
    }

    template <typename new_storage_mode>
    constexpr Polynomial<T, poly_order, new_storage_mode> convert() const
    {
        if constexpr (std::is_same_v<storage_mode, new_storage_mode>)
            return *this;

        Polynomial<T, poly_order, new_storage_mode> new_poly {};
        std::reverse_copy (coeffs.begin(), coeffs.end(), new_poly.coeffs.begin());
        return new_poly;
    }

    constexpr void set_coefficients (const std::array<T, num_coeffs>& new_coeffs)
    {
        coeffs = new_coeffs;
    }
};
} // namespace chowdsp

/** Useful methods for working with and evaluating polynomials */
namespace chowdsp::Polynomials
{
using chowdsp::poly_order_ascending;
using chowdsp::poly_order_descending;
using chowdsp::Polynomial;

/** Useful template type representing the product of two other types. */
template <typename T, typename X>
using P = decltype (T {} * X {});

/**
 * Evaluates a polynomial of a given order, using a naive method.
 * Coefficients should be given in the descending form.
 */
template <int poly_order, typename T, typename X>
constexpr P<T, X> naive (const Polynomial<T, poly_order, poly_order_descending>& poly, X x)
{
    CHOWDSP_USING_XSIMD_STD (pow);

    P<T, X> sum = poly.coeffs[poly_order];
    for (int n = 0; n < poly_order; ++n)
        sum += poly.coeffs[n] * pow (x, X (poly_order - n));

    return sum;
}

/**
 * Evaluates a polynomial of a given order, using a naive method.
 * Coefficients should be given in the ascending form.
 */
template <int poly_order, typename T, typename X>
constexpr P<T, X> naive (const Polynomial<T, poly_order, poly_order_ascending>& poly, X x)
{
    CHOWDSP_USING_XSIMD_STD (pow);

    P<T, X> sum = poly.coeffs[0];
    for (int n = 1; n <= poly_order; ++n)
        sum += poly.coeffs[n] * pow (x, static_cast<X> (n));

    return sum;
}

/**
 * Evaluates a polynomial of a given order, using Horner's method.
 * Coefficients should be given in descending form.
 * https://en.wikipedia.org/wiki/Horner%27s_method
 */
template <int poly_order, typename T, typename X>
constexpr P<T, X> horner (const Polynomial<T, poly_order, poly_order_descending>& poly, X x)
{
    P<T, X> b = poly.coeffs[0];
    for (int n = 1; n <= poly_order; ++n)
        b = b * x + poly.coeffs[n];

    return b;
}

/**
 * Evaluates a polynomial of a given order, using Horner's method.
 * Coefficients should be given in ascending form.
 * https://en.wikipedia.org/wiki/Horner%27s_method
 */
template <int poly_order, typename T, typename X>
constexpr P<T, X> horner (const Polynomial<T, poly_order, poly_order_ascending>& poly, X x)
{
    P<T, X> b = poly.coeffs.back();
    for (int n = poly_order; n > 0; --n)
        b = b * x + poly.coeffs[n - 1];

    return b;
}

/**
 * Evaluates a polynomial of a given order, using Estrin's scheme.
 * Coefficients should be given in descending form.
 * https://en.wikipedia.org/wiki/Estrin%27s_scheme
 */
template <int poly_order, typename T, typename X>
constexpr P<T, X> estrin (const Polynomial<T, poly_order, poly_order_descending>& poly, X x)
{
    if constexpr (poly_order <= 1) // base case
    {
        return poly.coeffs[1] + poly.coeffs[0] * x;
    }
    else
    {
        Polynomial<P<T, X>, poly_order / 2, poly_order_descending> temp {};
        for (int n = poly_order; n > 0; n -= 2)
            temp.coeffs[n / 2] = poly.coeffs[n] + poly.coeffs[n - 1] * x;

        if constexpr (poly_order % 2 == 0) // even order polynomial
            temp.coeffs[0] = poly.coeffs[0];

        return estrin<poly_order / 2> (temp, x * x); // recurse!
    }
}

/**
 * Evaluates a polynomial of a given order, using Estrin's scheme.
 * Coefficients should be given in descending form.
 * https://en.wikipedia.org/wiki/Estrin%27s_scheme
 */
template <int poly_order, typename T, typename X>
constexpr P<T, X> estrin (const Polynomial<T, poly_order, poly_order_ascending>& poly, X x)
{
    if constexpr (poly_order <= 1) // base case
    {
        return poly.coeffs[poly_order - 1] + poly.coeffs[poly_order] * x;
    }
    else
    {
        Polynomial<P<T, X>, poly_order / 2, poly_order_ascending> temp {};
        for (int n = 1; n <= poly_order; n += 2)
            temp.coeffs[n / 2] = poly.coeffs[n - 1] + poly.coeffs[n] * x;

        if constexpr (poly_order % 2 == 0) // even order polynomial
            temp.coeffs[poly_order / 2] = poly.coeffs[poly_order];

        return estrin<poly_order / 2> (temp, x * x); // recurse!
    }
}

/**
 * Computes the coefficients of the antiderivative of a polynomial.
 * Coefficients should be given in descending form.
 */
template <int poly_order, typename T>
constexpr Polynomial<T, poly_order + 1> antiderivative (const Polynomial<T, poly_order>& poly, T C = {})
{
    Polynomial<T, poly_order + 1> ad_poly {};
    for (int n = 0; n <= poly_order; ++n)
        ad_poly.coeffs[n] = poly.coeffs[n] / static_cast<T> (poly_order + 1 - n);
    ad_poly.coeffs[poly_order + 1] = C;

    return ad_poly;
}

/**
 * Computes the coefficients of the antiderivative of a polynomial.
 * Coefficients should be given in ascending form.
 */
template <int poly_order, typename T>
constexpr Polynomial<T, poly_order + 1, poly_order_ascending> antiderivative (const Polynomial<T, poly_order, poly_order_ascending>& poly, T C = {})
{
    Polynomial<T, poly_order + 1, poly_order_ascending> ad_poly {};
    ad_poly.coeffs[0] = C;
    for (int n = 1; n <= poly_order + 1; ++n)
        ad_poly.coeffs[n] = poly.coeffs[n - 1] / static_cast<T> (n);

    return ad_poly;
}

/**
 * Computes the coefficients of the derivative of a polynomial.
 * Coefficients should be given in descending form.
 */
template <int poly_order, typename T>
constexpr Polynomial<T, poly_order - 1> derivative (const Polynomial<T, poly_order>& poly)
{
    Polynomial<T, poly_order - 1> d_poly {};
    for (int n = 0; n < poly_order; ++n)
        d_poly.coeffs[n] = poly.coeffs[n] * static_cast<T> (poly_order - n);
    return d_poly;
}

/**
 * Computes the coefficients of the derivative of a polynomial.
 * Coefficients should be given in ascending form.
 */
template <int poly_order, typename T>
constexpr Polynomial<T, poly_order - 1, poly_order_ascending> derivative (const Polynomial<T, poly_order, poly_order_ascending>& poly)
{
    Polynomial<T, poly_order - 1, poly_order_ascending> d_poly {};
    for (int n = 0; n < poly_order; ++n)
        d_poly.coeffs[n] = poly.coeffs[n + 1] * static_cast<T> (n + 1);
    return d_poly;
}
} // namespace chowdsp::Polynomials

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
