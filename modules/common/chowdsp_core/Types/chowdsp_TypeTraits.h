#pragma once

/**
 * Creates a constexpr bool that checks if a class has the given static method.
 *
 * Usage:
 * @code {.cpp}
 * CHOWDSP_CHECK_HAS_STATIC_METHOD(HasToString, toString)
 * static_assert(HasToString<MyClass>, "MyClass must have a static toString method!");
 * @endcode
 */
#define CHOWDSP_CHECK_HAS_STATIC_METHOD(Name, Method)                                                                                   \
    template <typename _T>                                                                                                              \
    class Test_##Name                                                                                                                   \
    {                                                                                                                                   \
        using No = char;                                                                                                                \
        using Yes = long;                                                                                                               \
        static_assert (sizeof (No) != sizeof (Yes), "Yes and No have the same size on this platform, undefined behaviour will ensue!"); \
                                                                                                                                        \
        template <typename C>                                                                                                           \
        static Yes test (decltype (&C::Method));                                                                                        \
                                                                                                                                        \
        template <typename C>                                                                                                           \
        static No test (...);                                                                                                           \
                                                                                                                                        \
    public:                                                                                                                             \
        enum                                                                                                                            \
        {                                                                                                                               \
            value = sizeof (test<_T> (nullptr)) == sizeof (Yes)                                                                         \
        };                                                                                                                              \
    };                                                                                                                                  \
    template <typename _T>                                                                                                              \
    static constexpr bool Name = Test_##Name<_T>::value;

/**
 * Creates a constexpr bool that checks if a class has the given non-static method.
 *
 * Usage:
 * @code {.cpp}
 * CHOWDSP_CHECK_HAS_METHOD(HasGetAtIndex, getAtIndex, int{})
 * static_assert(HasGetAtIndex<MyClass>, "MyClass must have getAtIndex(int) method!");
 * @endcode
 */
#define CHOWDSP_CHECK_HAS_METHOD(Name, Method, ...)                                                                                     \
    template <typename _T>                                                                                                              \
    class Test_##Name                                                                                                                   \
    {                                                                                                                                   \
        using No = char;                                                                                                                \
        using Yes = long;                                                                                                               \
        static_assert (sizeof (No) != sizeof (Yes), "Yes and No have the same size on this platform, undefined behaviour will ensue!"); \
                                                                                                                                        \
        template <typename C>                                                                                                           \
        static Yes test (decltype (C().Method (__VA_ARGS__))*);                                                                         \
                                                                                                                                        \
        template <typename C>                                                                                                           \
        static No test (...);                                                                                                           \
                                                                                                                                        \
    public:                                                                                                                             \
        enum                                                                                                                            \
        {                                                                                                                               \
            value = sizeof (test<_T> (nullptr)) == sizeof (Yes)                                                                         \
        };                                                                                                                              \
    };                                                                                                                                  \
    template <typename _T>                                                                                                              \
    static constexpr bool Name = Test_##Name<_T>::value;

/**
 * Creates a constexpr bool that checks if a class has the given static member variable.
 *
 * Usage:
 * @code {.cpp}
 * CHOWDSP_CHECK_HAS_STATIC_MEMBER(HasName, name)
 * static_assert(HasName<MyClass>, "MyClass must have a static name method!");
 * @endcode
 */
#define CHOWDSP_CHECK_HAS_STATIC_MEMBER(Name, Member)                                                                                   \
    template <typename _T>                                                                                                              \
    class Test_##Name                                                                                                                   \
    {                                                                                                                                   \
        using No = char;                                                                                                                \
        using Yes = long;                                                                                                               \
        static_assert (sizeof (No) != sizeof (Yes), "Yes and No have the same size on this platform, undefined behaviour will ensue!"); \
                                                                                                                                        \
        template <typename C>                                                                                                           \
        static std::enable_if_t<std::is_member_pointer_v<decltype (&C::Member)>, No> test (decltype (C::Member)*);                      \
                                                                                                                                        \
        template <typename C>                                                                                                           \
        static std::enable_if_t<! std::is_member_pointer_v<decltype (&C::Member)>, Yes> test (decltype (C::Member)*);                   \
                                                                                                                                        \
        template <typename C>                                                                                                           \
        static No test (...);                                                                                                           \
                                                                                                                                        \
    public:                                                                                                                             \
        enum                                                                                                                            \
        {                                                                                                                               \
            value = sizeof (test<_T> (nullptr)) == sizeof (Yes)                                                                         \
        };                                                                                                                              \
    };                                                                                                                                  \
    template <typename _T>                                                                                                              \
    static constexpr bool Name = Test_##Name<_T>::value;

/**
 * Creates a constexpr bool that checks if a class has the given non-static member variable.
 *
 * Usage:
 * @code {.cpp}
 * CHOWDSP_CHECK_HAS_MEMBER(HasName, name)
 * static_assert(HasName<MyClass>, "MyClass must have `name` member variable!");
 * @endcode
 */
#define CHOWDSP_CHECK_HAS_MEMBER(Name, Member)                                                                                          \
    template <typename _T>                                                                                                              \
    class Test_##Name                                                                                                                   \
    {                                                                                                                                   \
        using No = char;                                                                                                                \
        using Yes = long;                                                                                                               \
        static_assert (sizeof (No) != sizeof (Yes), "Yes and No have the same size on this platform, undefined behaviour will ensue!"); \
                                                                                                                                        \
        template <typename C>                                                                                                           \
        static std::enable_if_t<std::is_member_pointer_v<decltype (&C::Member)>, Yes> test (decltype (C().Member)*);                    \
                                                                                                                                        \
        template <typename C>                                                                                                           \
        static std::enable_if_t<! std::is_member_pointer_v<decltype (&C::Member)>, No> test (decltype (C().Member)*);                   \
                                                                                                                                        \
        template <typename C>                                                                                                           \
        static No test (...);                                                                                                           \
                                                                                                                                        \
    public:                                                                                                                             \
        enum                                                                                                                            \
        {                                                                                                                               \
            value = sizeof (test<_T> (nullptr)) == sizeof (Yes)                                                                         \
        };                                                                                                                              \
    };                                                                                                                                  \
    template <typename _T>                                                                                                              \
    static constexpr bool Name = Test_##Name<_T>::value;

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
