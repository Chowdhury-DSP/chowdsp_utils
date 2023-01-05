#pragma once

#include "chowdsp_Version.h"

namespace chowdsp
{
#ifndef DOXYGEN
constexpr bool operator== (const Version& v1, const Version& v2) noexcept
{
    return v1.major == v2.major && v1.minor == v2.minor && v1.patch == v2.patch;
}

constexpr bool operator!= (const Version& v1, const Version& v2) noexcept
{
    return ! (v1 == v2);
}

constexpr bool operator> (const Version& v1, const Version& v2) noexcept
{
    return v1.major > v2.major
           || (v1.major == v2.major && v1.minor > v2.minor)
           || (v1.major == v2.major && v1.minor == v2.minor && v1.patch > v2.patch);
}

constexpr bool operator<(const Version& v1, const Version& v2) noexcept
{
    return v1.major < v2.major
           || (v1.major == v2.major && v1.minor < v2.minor)
           || (v1.major == v2.major && v1.minor == v2.minor && v1.patch < v2.patch);
}

constexpr bool operator>= (const Version& v1, const Version& v2) noexcept
{
    return v1 > v2 || v1 == v2;
}

constexpr bool operator<= (const Version& v1, const Version& v2) noexcept
{
    return v1 < v2 || v1 == v2;
}
#endif // DOXYGEN
} // namespace chowdsp