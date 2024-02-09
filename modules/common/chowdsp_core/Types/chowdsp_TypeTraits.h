#pragma once

namespace chowdsp
{
/** Helpers for evaluating type traits */
namespace TypeTraits
{
#ifndef DOXYGEN
    namespace is_iterable_detail
    {
        CHOWDSP_CHECK_HAS_METHOD (HasBegin, begin, )
        CHOWDSP_CHECK_HAS_METHOD (HasEnd, end, )

        template <typename T>
        static constexpr auto has_begin_end_v = HasBegin<T>&& HasEnd<T>;
    } // namespace is_iterable_detail
#endif // DOXYGEN

    /** True if type T can be iterated over like an STL container */
    template <typename T>
    static constexpr auto IsIterable = is_iterable_detail::has_begin_end_v<T>;

#ifndef DOXYGEN
    namespace is_vector_detail
    {
        template <typename T, typename U = void>
        struct is_insertable
        {
            static constexpr auto value = false;
        };

        template <typename T>
        struct is_insertable<T, std::void_t<typename T::value_type, typename T::const_iterator, decltype (std::declval<T&>().insert (std::declval<typename T::const_iterator>(), std::declval<typename T::value_type&&>()))>>
        {
            static constexpr auto value = true;
        };

        template <typename T, typename U = void>
        struct is_pushable
        {
            static constexpr auto value = false;
        };

        template <typename T>
        struct is_pushable<T, std::void_t<typename T::value_type, decltype (std::declval<T&>().push_back (std::declval<const typename T::value_type&>()))>>
        {
            static constexpr auto value = true;
        };

        template <typename T, typename U = void>
        struct is_popable
        {
            static constexpr auto value = false;
        };

        template <typename T>
        struct is_popable<T, std::void_t<decltype (std::declval<T&>().pop_back())>>
        {
            static constexpr auto value = true;
        };
    } // namespace is_vector_detail
#endif // DOXYGEN

    /** True if type T can be iterated over like an STL container */
    template <typename T>
    static constexpr auto IsVectorLike = IsIterable<T>&& is_vector_detail::is_insertable<T>::value&& is_vector_detail::is_pushable<T>::value&& is_vector_detail::is_popable<T>::value;

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

} // namespace TypeTraits

/** Return true if the complete type is a specialization of the outer type */
template <typename CompleteType, template <typename...> class OuterType>
struct is_specialization_of;

template <typename CompleteType, template <typename...> class OuterType>
struct is_specialization_of final : std::false_type
{
};

template <template <typename...> class OuterType, typename... TypeArgs>
struct is_specialization_of<OuterType<TypeArgs...>, OuterType> final : std::true_type
{
};

/** Return true if the complete type is a specialization of the outer type */
template <typename CompleteType, template <typename...> class OuterType>
static constexpr bool is_specialization_of_v = is_specialization_of<CompleteType, OuterType>::value;

template <typename T>
struct is_complete_type
{
    template <typename U>
    static auto test (U*) -> std::integral_constant<bool, sizeof (U) == sizeof (U)>;
    static auto test (...) -> std::false_type;
    static constexpr auto value = ! std::is_same_v<decltype (test (static_cast<T*> (nullptr))), std::false_type>;
};

template <typename T>
static constexpr auto is_complete_type_v = is_complete_type<T>::value;

/**
 * An empty struct intended to be used with std::conditional_t
 * to effectively "disable" some member of a struct/class.
 */
struct NullType
{
    template <typename... Ts>
    explicit NullType (Ts&&...)
    {
    }
};
} // namespace chowdsp
