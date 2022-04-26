#pragma once

#include <cmath>
#include <limits>

namespace chowdsp
{
/** Jacobi elliptic methods borrowmed from BOOST.math */
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

    template <class T>
    void jacobi_elliptic (const T& x, const T& k, T& sn, T& cn, T& dn)
    {
        jassert (k >= 0);

        if (k > 1)
        {
            T xp = x * k;
            T kp = 1 / k;
            jacobi_elliptic (xp, kp, sn, cn, dn);
            sn *= kp;
            return;
        }

        // Special cases first:
        if (x == 0)
        {
            cn = 1;
            dn = 1;
            sn = 0;
            return;
        }
        if (k == 0)
        {
            cn = std::cos (x);
            dn = 1;
            sn = std::sin (x);
            return;
        }
        if (k == 1)
        {
            cn = dn = 1 / std::cosh (x);
            sn = std::tanh (x);
            return;
        }

        // Asymptotic forms from A&S 16.13:
        if (k < std::pow (std::numeric_limits<T>::epsilon(), 0.25))
        {
            T su = std::sin (x);
            T cu = std::cos (x);
            T m = k * k;
            dn = 1 - m * su * su / 2;
            cn = cu + m * (x - su * cu) * su / 4;
            sn = su - m * (x - su * cu) * cu / 4;
            return;
        }

        T T1;
        T kc = 1 - k;
        T k_prime = k < 0.5 ? T (std::sqrt (1 - k * k)) : T (std::sqrt (2 * kc - kc * kc));
        T T0 = jacobi_recurse (x, k, T (1), k_prime, 0, &T1);
        cn = std::cos (T0);
        dn = std::cos (T0) / std::cos (T1 - T0);
        sn = std::sin (T0);
    }
} // namespace jacobi
} // namespace chowdsp
