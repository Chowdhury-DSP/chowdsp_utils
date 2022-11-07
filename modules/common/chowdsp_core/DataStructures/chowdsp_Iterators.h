#pragma once

namespace chowdsp
{
// N.B. `enumerate` and `zip` implementations are borrowed from
// https://github.com/surge-synthesizer/sst-cpputils, under the MIT license.

/*
 * enumerate allows structured bindings of iterators. A typical usage would be
 *
 * ```
 * std::vector<int> v {7,14,21};
 * for (const auto [idx, val] : chowdsp::enumerate (v))
 * {
 *     assert ((idx + 1) * 7 == val);
 * }
 * ```
 *
 * This code comes from this very useful blog entry.
 * https://www.reedbeta.com/blog/python-like-enumerate-in-cpp17/
 */
template <typename T, typename TIter = decltype (std::begin (std::declval<T>())), typename = decltype (std::end (std::declval<T>()))>
constexpr auto enumerate (T&& iterable)
{
    struct iterator
    {
        size_t i;
        TIter iter;
        bool operator!= (const iterator& other) const { return iter != other.iter; }
        void operator++()
        {
            ++i;
            ++iter;
        }
        auto operator*() const { return std::tie (i, *iter); }
    };
    struct iterable_wrapper
    {
        T iterable;
        auto begin() { return iterator { 0, std::begin (iterable) }; }
        auto end() { return iterator { 0, std::end (iterable) }; }
    };
    return iterable_wrapper { std::forward<T> (iterable) };
}

/*
 * Zip is an obvious extension of the above, interleaving a pair of containers.
 * We use the semantic that we stop at the end of the shortest container.
 *
 * ```
 * std::vector<int> v0 {0,2,4};
 * for (const auto& [a, b] : chowdsp::zip (v0, "ace ventura"))
 * {
 *   REQUIRE (a + 'a' == b);
 * }
 * ```
 *
 * This code is motivated by the above enumerate, but separate obviously
 */
template <typename T, typename S, typename TIter = decltype (std::begin (std::declval<T>())), typename = decltype (std::end (std::declval<T>())), typename SIter = decltype (std::begin (std::declval<S>())), typename = decltype (std::end (std::declval<S>()))>
constexpr auto zip (T&& iterableT, S&& iterableS)
{
    struct iterator
    {
        TIter titer;
        SIter siter;

        const TIter tend;
        const SIter send;
        bool operator!= (const iterator& other) const
        {
            return titer != other.titer && siter != other.siter;
        }
        void operator++()
        {
            ++siter;
            ++titer;

            if (titer == tend)
                siter = send;
            else if (siter == send)
                titer = tend;
        }
        auto operator*() const { return std::tie (*titer, *siter); }
    };
    struct iterable_wrapper
    {
        T titerable;
        S siterable;
        auto begin()
        {
            return iterator { std::begin (titerable), std::begin (siterable), std::end (titerable), std::end (siterable) };
        }
        auto end()
        {
            return iterator { std::end (titerable), std::end (siterable), std::end (titerable), std::end (siterable) };
        }
    };
    return iterable_wrapper { std::forward<T> (iterableT), std::forward<S> (iterableS) };
}
} // namespace chowdsp
