#pragma once

namespace nlohmann
{
template <>
struct adl_serializer<juce::String>
{
    static void to_json (json& j, const juce::String& s)
    {
        j = s.toStdString();
    }

    static void from_json (const json& j, juce::String& s)
    {
        s = j.get<std::string>();
    }
};
} // namespace nlohmann

namespace chowdsp
{
using json = nlohmann::json;

/**
 * nlohmann::json is much easier to use than JUCE's JSON API.
 *
 * Here are some wrappers making it easier to combine the two!
 */
namespace JSONUtils
{
    inline json fromFile (const juce::File& file)
    {
        juce::FileInputStream jsonInputStream { file };
        return json::parse (jsonInputStream.readEntireStreamAsString().toStdString());
    }

    inline void toFile (const json& j, const juce::File& file)
    {
        file.replaceWithText (j.dump());
    }

    inline bool isSameType (const json& j1, const json& j2)
    {
        return (j1.is_number() && j2.is_number()) || (j1.type() == j2.type());
    }
} // namespace JSONUtils

} // namespace chowdsp
