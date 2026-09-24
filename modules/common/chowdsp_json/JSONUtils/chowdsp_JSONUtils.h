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
    inline json fromBinaryData (const void* data, int dataSize) // NOSONAR (needs void* to be compatible with JUCE BinaryData)
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

    /** Load a json object from a MemoryBlock */
    inline json fromMemoryBlock (const juce::MemoryBlock& block)
    {
        juce::MemoryInputStream jsonInputStream { block, false };
        return fromInputStream (jsonInputStream);
    }

    /** Dump a json object to an output stream */
    inline void toOutputStream (const json& j, juce::OutputStream& stream, const int indent = -1, const char indent_char = ' ')
    {
        const auto text = j.dump (indent, indent_char);
        stream.write (text.data(), (size_t) text.size());
    }

    /** Dump a json object to a file */
    inline void toFile (const json& j, const juce::File& file, const int indent = -1, const char indent_char = ' ')
    {
        if (! file.deleteFile())
        {
            // unable to delete existing file (maybe there's a directory there?)
            jassertfalse;
            return;
        }

        if (auto jsonOutputStream = file.createOutputStream())
        {
            toOutputStream (j, *jsonOutputStream, indent, indent_char);
        }
        else
        {
            // LCOV_EXCL_START
            // unable to create an output stream for this file
            jassertfalse;
            // LCOV_EXCL_END
        }
    }

    /** Dump a json object to a MemoryBlock */
    inline void toMemoryBlock (const json& j, juce::MemoryBlock& block)
    {
        juce::MemoryOutputStream jsonOutputStream { block, true };
        toOutputStream (j, jsonOutputStream);
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
