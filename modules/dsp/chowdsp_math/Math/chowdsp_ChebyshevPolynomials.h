#pragma once

namespace chowdsp
{
#ifndef DOXYGEN
namespace cheby_detail
{
    template <typename T, int max_order = 32>
    constexpr auto cheby0()
    {
        Polynomial<T, max_order, poly_order_ascending> poly {};
        poly.coeffs[0] = static_cast<T> (1);
        return poly;
    }

    template <typename T, int max_order = 32>
    constexpr auto cheby1()
    {
        Polynomial<T, max_order, poly_order_ascending> poly {};
        poly.coeffs[1] = static_cast<T> (1);
        return poly;
    }
} // namespace cheby_detail
#endif

template <typename T, int max_order = 32>
constexpr auto chebyshev_polynomial_recurse (const Polynomial<T, max_order, poly_order_ascending>& cheby_n1,
                                             const Polynomial<T, max_order, poly_order_ascending>& cheby_n2)
{
    Polynomial<T, max_order, poly_order_ascending> cheby_n {};
    cheby_n.coeffs[0] = -cheby_n2.coeffs[0];
    for (size_t i = 1; i < (size_t) max_order; ++i)
        cheby_n.coeffs[i] = (T) 2 * cheby_n1.coeffs[i - 1] - cheby_n2.coeffs[i];
    return cheby_n;
}

template <typename T, int order, int max_order = 32>
constexpr auto chebyshev_polynomial()
{
    if constexpr (order == 0)
        return cheby_detail::cheby0<T, 32>();

    if constexpr (order == 1)
        return cheby_detail::cheby1<T, 32>();

    auto cheby_n2 = cheby_detail::cheby0<T, 32>();
    auto cheby_n1 = cheby_detail::cheby1<T, 32>();
    auto cheby_n = Polynomial<T, max_order, poly_order_ascending> {};
    for (int i = 2; i <= order; ++i)
    {
        cheby_n = chebyshev_polynomial_recurse<T, max_order> (cheby_n1, cheby_n2);
        cheby_n2 = cheby_n1;
        cheby_n1 = cheby_n;
    }

    return cheby_n;
}
} // namespace chowdsp
