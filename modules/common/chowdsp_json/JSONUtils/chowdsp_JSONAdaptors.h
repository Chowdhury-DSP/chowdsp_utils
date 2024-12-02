#pragma once

#if JUCE_MODULE_AVAILABLE_chowdsp_version
#include <chowdsp_version/chowdsp_version.h>
#endif

#ifndef DOXYGEN
namespace nlohmann
{
/** Adapter so that nlohmann::json can serialize juce::String */
template <>
struct adl_serializer<juce::String>
{
    static void to_json (json& j, const juce::String& s)
    {
        j = s.toUTF8();
    }

    static void from_json (const json& j, juce::String& s)
    {
        s = j.get<std::string>();
    }
};

#if JUCE_MODULE_AVAILABLE_chowdsp_version
/** Adapter so that nlohmann::json can serialize chowdsp::Version */
template <>
struct adl_serializer<::chowdsp::Version>
{
    static void to_json (json& j, const ::chowdsp::Version& version)
    {
        j = version.getVersionHint();
    }

    static void from_json (const json& j, ::chowdsp::Version& version)
    {
        if (! j.is_number_integer())
        {
            version = {};
            return;
        }

        const auto versionHint = j.get<int>();
        const auto major = versionHint / 10000;
        const auto minor = (versionHint % 10000) / 100;
        const auto patch = versionHint % 100;
        version = { major, minor, patch };
    }
};
#endif
} // namespace nlohmann
#endif
