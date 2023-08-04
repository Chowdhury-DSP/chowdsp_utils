#pragma once

#include <cmath>
#include <limits>

namespace chowdsp
{
/** Jacobi elliptic methods mostly adapted from BOOST.math */
namespace jacobi
{
    template <class T>
    T jacobi_recurse (const T& x, const T& k, T anm1, T bnm1, unsigned N, T* pTn)
    {
        ++N;
        T Tn;
        T cn = (anm1 - bnm1) / 2;
        T an = (anm1 + bnm1) / 2;
        if (cn < std::numeric_limits<T>::epsilon())
        {
            Tn = std::ldexp (T (1), (int) N) * x * an;
        }
        else
            Tn = jacobi_recurse<T> (x, k, an, std::sqrt (anm1 * bnm1), N, nullptr);
        if (pTn)
            *pTn = Tn;
        return (Tn + std::asin ((cn / an) * std::sin (Tn))) / 2;
    }

    /**
     * The function jacobi_elliptic calculates the three copolar Jacobi elliptic functions
     * sn(u, k), cn(u, k) and dn(u, k).
     *
     * This method gives equivalent output to scipy.special.ellipj
     */
    template <class T>
    std::tuple<T, T, T> jacobi_elliptic (T x, T k)
    {
        jassert (k >= (T) 0);

        if (k > (T) 1)
        {
            T xp = x * k;
            T kp = (T) 1 / k;
            auto [sn, cn, dn] = jacobi_elliptic (xp, kp);
            sn *= kp;
            return std::make_tuple (sn, cn, dn);
        }

        // this step is not present in the Boost implementations but is
        // needed to get the same output as the scipy implementation.
        k = std::sqrt (k);

        // Special cases first:
        if (juce::exactlyEqual (x, (T) 0))
        {
            return std::make_tuple ((T) 0, (T) 1, (T) 1);
        }
        if (juce::exactlyEqual (k, (T) 0))
        {
            return std::make_tuple (std::sin (x), std::cos (x), (T) 1);
        }
        if (juce::exactlyEqual (k, (T) 1))
        {
            const auto cn_dn = (T) 1 / std::cosh (x);
            return std::make_tuple (std::tanh (x), cn_dn, cn_dn);
        }

        // Asymptotic forms from A&S 16.13:
        if (k < std::pow (std::numeric_limits<T>::epsilon(), 0.25))
        {
            T su = std::sin (x);
            T cu = std::cos (x);
            T m = k * k;
            const auto dn = 1 - m * su * su / 2;
            const auto cn = cu + m * (x - su * cu) * su / 4;
            const auto sn = su - m * (x - su * cu) * cu / 4;
            return std::make_tuple (sn, cn, dn);
        }

        T T1;
        T kc = 1 - k;
        T k_prime = k < 0.5 ? T (std::sqrt (1 - k * k)) : T (std::sqrt (2 * kc - kc * kc));
        T T0 = jacobi_recurse (x, k, T (1), k_prime, 0, &T1);
        const auto cn = std::cos (T0);
        const auto dn = std::cos (T0) / std::cos (T1 - T0);
        const auto sn = std::sin (T0);
        return std::make_tuple (sn, cn, dn);
    }
} // namespace jacobi
} // namespace chowdsp
