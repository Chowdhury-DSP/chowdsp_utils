#pragma once

/** Checks if a class has a static method */
#define CHOWDSP_CHECK_HAS_STATIC_METHOD(Name, Class, Method)                                                                            \
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
            value = sizeof (test<Class> (nullptr)) == sizeof (Yes)                                                                      \
        };                                                                                                                              \
    };                                                                                                                                  \
    static constexpr bool Name = Test_##Name::value;
