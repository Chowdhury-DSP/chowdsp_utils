#pragma once

#include <catch2/catch2.hpp>

// Helper for using Catch's REQUIRE with a custom failure message
#define REQUIRE_MESSAGE(cond, msg) \
    do                             \
    {                              \
        INFO (msg);                \
        REQUIRE (cond);            \
    } while ((void) 0, 0)
