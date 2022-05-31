#pragma once

namespace chowdsp
{
/** Helpers for tempalte meta-programming */
namespace MetaHelpers
{
#ifndef DOXYGEN
    namespace is_container_detail
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
    } // namespace is_container_detail
#endif // DOXYGEN

    /** True if type T is a STL-compatible container */
    template <typename T>
    static constexpr auto IsContainer = is_container_detail::has_begin_end_v<T>;

} // namespace MetaHelpers
} // namespace chowdsp
