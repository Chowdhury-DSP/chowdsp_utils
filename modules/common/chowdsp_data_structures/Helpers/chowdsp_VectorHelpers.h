#pragma once

#include <vector>

namespace chowdsp
{

/** Inserts an element into a sorted vector, with a custom comparison operator. */
template <typename Vector, typename Pred, typename T = typename Vector::value_type, typename = typename std::enable_if_t<TypeTraits::IsVectorLike<Vector>, void>>
auto insert_sorted (Vector& vec, T&& item, Pred pred)
{
    return vec.insert (std::upper_bound (vec.begin(), vec.end(), item, pred), item);
}

/** Inserts an element into a sorted vector, with the standard comparison operator. */
template <typename Vector, typename T = typename Vector::value_type, typename = typename std::enable_if_t<TypeTraits::IsVectorLike<Vector>, void>>
auto insert_sorted (Vector& vec, T&& item)
{
    return vec.insert (std::upper_bound (vec.begin(), vec.end(), item, std::less<T> {}), item);
}

template <typename Vector, class U, typename = typename std::enable_if_t<TypeTraits::IsVectorLike<Vector>, void>>
constexpr auto erase (Vector& c, const U& value)
{
    auto it = std::remove (c.begin(), c.end(), value);
    auto r = std::distance (it, c.end());
    c.erase (it, c.end());
    return (size_t) r;
}

template <typename Vector, typename Pred, typename = typename std::enable_if_t<TypeTraits::IsVectorLike<Vector>, void>>
constexpr auto erase_if (Vector& c, Pred pred)
{
    auto it = std::remove_if (c.begin(), c.end(), pred);
    auto r = std::distance (it, c.end());
    c.erase (it, c.end());
    return (size_t) r;
}

/** Helpers for working with std::vector */
namespace VectorHelpers
{
    // We want these methods to live in the chowdsp namespace now,
    // but don't want to break backwards compatibility.

    using ::chowdsp::erase;
    using ::chowdsp::erase_if;
    using ::chowdsp::insert_sorted;
} // namespace VectorHelpers
} // namespace chowdsp
