#pragma once

#include <vector>

namespace chowdsp
{
/** Helpers for working with std::vector */
namespace VectorHelpers
{
    /** Inserts an element into a sorted vector, with a custom comparison operator. */
    template <typename T, typename Alloc, typename Pred>
    typename std::vector<T, Alloc>::iterator insert_sorted (std::vector<T, Alloc>& vec, T&& item, Pred pred)
    {
        return vec.insert (std::upper_bound (vec.begin(), vec.end(), item, pred), item);
    }

    /** Inserts an element into a sorted vector, with the standard comparison operator. */
    template <typename T, typename Alloc>
    typename std::vector<T, Alloc>::iterator insert_sorted (std::vector<T, Alloc>& vec, T&& item)
    {
        return vec.insert (std::upper_bound (vec.begin(), vec.end(), item, std::less<T> {}), item);
    }

#if __cpp_lib_erase_if
    using std::erase_if, std::erase;
#else
    template <class T, class Alloc, class U>
    constexpr typename std::vector<T, Alloc>::size_type
        erase (std::vector<T, Alloc>& c, const U& value)
    {
        auto it = std::remove (c.begin(), c.end(), value);
        auto r = std::distance (it, c.end());
        c.erase (it, c.end());
        return (size_t) r;
    }

    template <class T, class Alloc, class Pred>
    constexpr typename std::vector<T, Alloc>::size_type
        erase_if (std::vector<T, Alloc>& c, Pred pred)
    {
        auto it = std::remove_if (c.begin(), c.end(), pred);
        auto r = std::distance (it, c.end());
        c.erase (it, c.end());
        return (size_t) r;
    }
#endif
} // namespace VectorHelpers
} // namespace chowdsp
