/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            chowdsp_version
   vendor:        Chowdhury DSP
   version:       2.0.0
   name:          ChowDSP Plugin Versioning
   description:   Versioning system for ChowDSP plugins
   dependencies:  juce_core

   website:       https://ccrma.stanford.edu/~jatin/chowdsp
   license:       BSD 3-clause

   END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>

namespace chowdsp
{
#ifndef DOXYGEN
namespace version_detail
{
    /** Borrowed from: https://stackoverflow.com/questions/25195176/how-do-i-convert-a-c-string-to-a-int-at-compile-time */
    constexpr int stoi (std::string_view str, std::size_t* pos = nullptr)
    {
        using namespace std::literals;
        const auto numbers = "0123456789"sv;

        const auto begin = str.find_first_of (numbers);
        if (begin == std::string_view::npos)
            throw std::invalid_argument { "stoi" };

        const auto sign = begin != 0U && str[begin - 1U] == '-' ? -1 : 1;
        str.remove_prefix (begin);

        const auto end = str.find_first_not_of (numbers);
        if (end != std::string_view::npos)
            str.remove_suffix (std::size (str) - end);

        auto result = 0;
        auto multiplier = 1;
        for (auto i = static_cast<ptrdiff_t> (std::size (str) - 1U); i >= 0; --i)
        {
            auto number = (int) *(str.data() + i) - '0';
            result += number * multiplier * sign;
            multiplier *= 10;
        }

        if (pos != nullptr)
            *pos = begin + std::size (str);
        return result;
    }
} // namespace version_detail
#endif // DOXYGEN

/** Utility class to manage version strings. */
class Version
{
public:
    explicit Version (const juce::String& versionStr = "0.0.0");

    constexpr explicit Version (const std::string_view& versionStr = "0.0.0")
    {
        int numDots = 0;
        for (auto ch : versionStr)
            if (ch == '.')
                numDots++;

        // Valid version strings must have two dots!
        jassert (numDots == 2);

        const auto firstDot = versionStr.find ('.');
        const auto majorVersionStr = versionStr.substr (0, firstDot);
        major = version_detail::stoi (majorVersionStr);

        auto remainder = versionStr.substr (firstDot + 1);
        const auto secondDot = remainder.find ('.');
        const auto minorVersionStr = remainder.substr (0, secondDot);
        minor = version_detail::stoi (minorVersionStr);

        const auto patchVersionStr = remainder.substr (secondDot + 1);
        patch = version_detail::stoi (patchVersionStr);
    }

    constexpr Version (const Version&) = default;
    constexpr Version& operator= (const Version&) = default;

    [[nodiscard]] juce::String getVersionString() const;

    friend constexpr bool operator== (const Version& v1, const Version& v2) noexcept;
    friend constexpr bool operator!= (const Version& v1, const Version& v2) noexcept;
    friend constexpr bool operator> (const Version& v1, const Version& v2) noexcept;
    friend constexpr bool operator<(const Version& v1, const Version& v2) noexcept;
    friend constexpr bool operator>= (const Version& v1, const Version& v2) noexcept;
    friend constexpr bool operator<= (const Version& v1, const Version& v2) noexcept;

private:
    int major = 0;
    int minor = 0;
    int patch = 0;
};

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

/** Tools for working with software versioning. */
namespace VersionUtils
{
    /** Utility class to manage version strings. */
    using Version = ::chowdsp::Version;
} // namespace VersionUtils
} // namespace chowdsp
