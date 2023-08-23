#pragma once

namespace chowdsp
{
// N.B. `enumerate` and `zip` implementations are borrowed from
// https://github.com/surge-synthesizer/sst-cpputils, under the MIT license.

/**
 * Enumerate allows structured bindings of iterators. A typical usage would be
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

/**
 * Zip is an obvious extension of enumerate, interleaving a pair of containers.
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

/** Iterates over a container in reverse */
template <typename T>
class reverse
{
private:
    T& iterable_;

public:
    explicit reverse (T& iterable) : iterable_ { iterable } {}
    [[nodiscard]] auto begin() const { return std::rbegin (iterable_); }
    [[nodiscard]] auto end() const { return std::rend (iterable_); }
};

#ifndef DOXYGEN
namespace zip_multi_detail
{
    template <typename T>
    struct make_iter
    {
        using type = decltype (std::begin (std::declval<T&>()));
    };

    template <typename... T>
    struct make_iters
    {
        using type = std::tuple<typename make_iter<T>::type...>;
    };

    template <typename... T>
    using make_iters_t = typename make_iters<T...>::type;

    template <typename... T, size_t... I>
    auto make_refs (const std::tuple<T...>& t, std::index_sequence<I...>)
    {
        return std::tie (*std::get<I> (t)...);
    }

    template <typename... T>
    auto make_refs (const std::tuple<T...>& t)
    {
        return make_refs<T...> (t, std::make_index_sequence<sizeof...(T)> {});
    }

    template <typename... T, size_t... I>
    auto make_begin_iters (const std::tuple<T...>& t, std::index_sequence<I...>)
    {
        return std::make_tuple (std::begin (std::get<I> (t))...);
    }

    template <typename... T>
    auto make_begin_iters (const std::tuple<T...>& t)
    {
        return make_begin_iters<T...> (t, std::make_index_sequence<sizeof...(T)> {});
    }

    template <typename... T, size_t... I>
    auto make_end_iters (const std::tuple<T...>& t, std::index_sequence<I...>)
    {
        return std::make_tuple (std::end (std::get<I> (t))...);
    }

    template <typename... T>
    auto make_end_iters (const std::tuple<T...>& t)
    {
        return make_end_iters<T...> (t, std::make_index_sequence<sizeof...(T)> {});
    }
} // namespace zip_multi_detail
#endif // DOXYGEN

/**
 * Similar to iterators::zip(), except that it supports iterating over
 * more than 2 containers at a time.
 */
template <typename... T, typename Iters = typename zip_multi_detail::make_iters<T...>::type>
constexpr auto zip_multi (T&... iterables)
{
    struct iterator
    {
        Iters iters;
        bool operator!= (const iterator& other) const
        {
            bool areAnyEqual = false;
            TupleHelpers::forEachInTuple (
                [&] (const auto& iter, auto idx)
                {
                    areAnyEqual |= (iter == std::get<idx> (other.iters));
                },
                iters);
            return ! areAnyEqual;
        }
        void operator++()
        {
            TupleHelpers::forEachInTuple ([] (auto& iter, size_t)
                                          { ++iter; },
                                          iters);
        }
        auto operator*() const { return zip_multi_detail::make_refs (iters); }
    };
    struct iterable_wrapper
    {
        std::tuple<T&...> titerable;
        auto begin()
        {
            return iterator { zip_multi_detail::make_begin_iters (titerable) };
        }
        auto end()
        {
            return iterator { zip_multi_detail::make_end_iters (titerable) };
        }
    };
    return iterable_wrapper { std::tie (iterables...) };
}
} // namespace chowdsp
