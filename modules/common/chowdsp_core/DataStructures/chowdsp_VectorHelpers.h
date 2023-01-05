#pragma once

#include <vector>

namespace chowdsp
{
/** Helpers for working with std::vector */
namespace VectorHelpers
{
    /** Inserts an element into a sorted vector, with a custom comparison operator. */
    template <typename T, typename Pred>
    typename std::vector<T>::iterator insert_sorted (std::vector<T>& vec, T&& item, Pred pred)
    {
        return vec.insert (std::upper_bound (vec.begin(), vec.end(), item, pred), item);
    }

    /** Inserts an element into a sorted vector, with the standard comparison operator. */
    template <typename T>
    typename std::vector<T>::iterator insert_sorted (std::vector<T>& vec, T&& item)
    {
        return vec.insert (std::upper_bound (vec.begin(), vec.end(), item, std::less<T> {}), item);
    }
} // namespace VectorHelpers
} // namespace chowdsp
