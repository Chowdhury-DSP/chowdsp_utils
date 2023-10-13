#pragma once

/**
 * Creates a constexpr bool that checks if a class has the given static method.
 *
 * Usage:
 * @code {.cpp}
 * CHOWDSP_CHECK_HAS_STATIC_METHOD(HasToString, toString, std::declval<int>())
 * static_assert(HasToString<MyClass>, "MyClass must have a static toString(int) method!");
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
 * CHOWDSP_CHECK_HAS_METHOD(HasGetAtIndex, getAtIndex, std::declval<int>())
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
        static Yes test (decltype (std::declval<C>().Method (__VA_ARGS__))*);                                                           \
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
 * static_assert(HasName<MyClass>, "MyClass must have a static name member!");
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
        static std::enable_if_t<std::is_member_pointer_v<decltype (&C::Member)>, Yes> test (decltype (std::declval<C>().Member)*);      \
                                                                                                                                        \
        template <typename C>                                                                                                           \
        static std::enable_if_t<! std::is_member_pointer_v<decltype (&C::Member)>, No> test (decltype (std::declval<C>().Member)*);     \
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
