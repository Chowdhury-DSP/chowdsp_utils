#pragma once

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
} // namespace nlohmann
#endif
