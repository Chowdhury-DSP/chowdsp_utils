#pragma once

namespace chowdsp
{
/** Helpers for tempalte meta-programming */
namespace MetaHelpers
{
#ifndef DOXYGEN
    namespace is_iterable_detail
    {
        template <typename T>
        struct has_begin_end
        {
            typedef char yes;
            typedef long no;

            template <typename C>
            static yes test_begin (decltype (C().begin())*);

            template <typename C>
            static no test_begin (...);

            template <typename C>
            static yes test_end (decltype (C().end())*);

            template <typename C>
            static no test_end (...);

            static constexpr bool begin_value = sizeof (test_begin<T> (nullptr)) == sizeof (yes);
            static constexpr bool end_value = sizeof (test_end<T> (nullptr)) == sizeof (yes);
        };

        template <typename T>
        static constexpr auto has_begin_end_v = has_begin_end<T>::begin_value&& has_begin_end<T>::end_value;
    } // namespace is_iterable_detail
#endif // DOXYGEN

    /** True if type T can be iterated over like an STL container */
    template <typename T>
    static constexpr auto IsIterable = is_iterable_detail::has_begin_end_v<T>;

#ifndef DOXYGEN
    namespace is_maplike_detail
    {
        template <typename T, typename U = void>
        struct is_mappish_impl
        {
            static constexpr auto value = false;
        };

        template <typename T>
        struct is_mappish_impl<T, std::void_t<typename T::key_type, typename T::mapped_type, decltype (std::declval<T&>()[std::declval<const typename T::key_type&>()])>>
        {
            static constexpr auto value = true;
        };
    } // namespace is_maplike_detail
#endif // DOXYGEN

    /** True if the given type is map-like (i.e. uses key-value pairs) */
    template <typename T>
    static constexpr auto IsMapLike = is_maplike_detail::is_mappish_impl<T>::value;

} // namespace MetaHelpers
} // namespace chowdsp
