#pragma once

namespace chowdsp
{
/** Useful methods for interfacing between JUCE and nlohmann::json */
namespace JSONUtils
{
    /** Load a json object from a juce::InputStream */
    inline json fromInputStream (juce::InputStream& stream)
    {
        return json::parse (stream.readEntireStreamAsString().toStdString());
    }

    /** Load a json object from binary data */
    inline json fromBinaryData (const void* data, int dataSize)
    {
        juce::MemoryInputStream jsonInputStream { data, (size_t) dataSize, false };
        return fromInputStream (jsonInputStream);
    }

    /** Load a json object from a file */
    inline json fromFile (const juce::File& file)
    {
        juce::FileInputStream jsonInputStream { file };
        return fromInputStream (jsonInputStream);
    }

    /** Dump a json object to a file */
    inline void toFile (const json& j, const juce::File& file)
    {
        // You've gotta create the file before writing JSON to it!
        jassert (file.existsAsFile());
        file.replaceWithText (j.dump());
    }

    /**
 * Check if two json objects have the same type.
 * Note that all number types are treated as the same.
 */
    inline bool isSameType (const json& j1, const json& j2)
    {
        return (j1.is_number() && j2.is_number()) || (j1.type() == j2.type());
    }
} // namespace JSONUtils
} // namespace chowdsp
