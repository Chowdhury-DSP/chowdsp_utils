#pragma once

namespace chowdsp
{
/** Implementations of methods commonly used in combinatorics */
namespace Combinatorics
{
    /** Computes the factorial of n using recursion */
    constexpr int factorial (int x)
    {
        return x <= 1 ? 1 : x * factorial (x - 1);
    }

    /** Computes the permutation of n and k */
    constexpr int permutation (int n, int k)
    {
        return factorial (n) / factorial (n - k);
    }

    /** Computes the combination of n and k ("n choose k") */
    constexpr int combination (int n, int k)
    {
        return permutation (n, k) / factorial (k);
    }
} // namespace Combinatorics
} // namespace chowdsp
