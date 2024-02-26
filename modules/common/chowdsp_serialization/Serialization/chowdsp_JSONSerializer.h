#pragma once

namespace chowdsp
{
/** Serializer which serializes data into a JSON format */
class JSONSerializer : public BaseSerializer
{
public:
    using SerializedType = json;
    using DeserializedType = const json&;

    static juce::String toString (const json& serial)
    {
        return serial.dump();
    }

    static void toFile (const json& serial, const juce::File& file)
    {
        JSONUtils::toFile (serial, file);
    }

    static void toMemoryBlock (const json& serial, juce::MemoryBlock& block)
    {
        JSONUtils::toMemoryBlock (serial, block);
    }

    static SerializedType fromFile (const juce::File& file)
    {
        try
        {
            return JSONUtils::fromFile (file);
        }
        catch (...)
        {
            jassertfalse; // unable to load file!
            return {};
        }
    }

    static SerializedType fromMemoryBlock (const juce::MemoryBlock& block)
    {
        try
        {
            return JSONUtils::fromMemoryBlock (block);
        }
        catch (...)
        {
            jassertfalse; // unable to load file!
            return {};
        }
    }

    static SerializedType fromBinaryData (const void* data, int dataSize)
    {
        try
        {
            return JSONUtils::fromBinaryData (data, dataSize);
        }
        catch (...)
        {
            jassertfalse; // unable to load from data!
            return {};
        }
    }

    static auto createBaseElement()
    {
        return json::array();
    }

    static void addChildElement (json& parent, json&& newChild)
    {
        parent.push_back (std::move (newChild));
    }

    /** If no child element is available, this "empty" JSON object will be returned by default. */
    inline static const json defaultJsonDeserial {};

    static const auto& getChildElement (const json& parent, int index)
    {
        if (! juce::isPositiveAndBelow (index, parent.size()))
        {
            jassertfalse;
            return defaultJsonDeserial;
        }

        return parent[(size_t) index];
    }

    static int getNumChildElements (const json& serial)
    {
        if (! serial.is_array())
        {
            jassertfalse;
            return 0;
        }

        return (int) serial.size();
    }

    template <typename T>
    static T serializeArithmeticType (T x)
    {
        return x;
    }

    template <typename T>
    static T deserializeArithmeticType (const json& serial)
    {
        if (! JSONUtils::isSameType (serial, T {}))
        {
            jassertfalse;
            return T {};
        }

        return serial.get<T>();
    }

    template <typename T>
    static T serializeEnumType (T x)
    {
        return serializeArithmeticType (x);
    }

    template <typename T>
    static T deserializeEnumType (const json& serial)
    {
        return deserializeArithmeticType<T> (serial);
    }

    template <typename T>
    static const T& serializeString (const T& x)
    {
        return x;
    }

    template <typename T>
    static T deserializeString (const json& serial)
    {
        if (! JSONUtils::isSameType (serial, T {}))
        {
            jassertfalse;
            return T {};
        }

        return serial.get<T>();
    }

private:
    JSONSerializer() = default; // static use only
};
} // namespace chowdsp
