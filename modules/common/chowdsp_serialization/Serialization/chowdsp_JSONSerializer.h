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

    static SerializedType fromFile (const juce::File& file)
    {
        return JSONUtils::fromFile (file);
    }

    static SerializedType fromBinaryData (const void* data, int dataSize)
    {
        return JSONUtils::fromBinaryData (data, dataSize);
    }

    static auto createBaseElement()
    {
        return json::array();
    }

    static void addChildElement (json& parent, json&& newChild)
    {
        parent.push_back (newChild);
    }

    static auto getChildElement (const json& parent, int index)
    {
        return parent[(size_t) index];
    }

    static int getNumChildElements (const json& serial)
    {
        if (serial.is_array())
            return (int) serial.size();

        return 0;
    }

    template <typename T>
    static T serializeArithmeticType (T x)
    {
        return x;
    }

    template <typename T>
    static T deserializeArithmeticType (const json& serial)
    {
        return serial.get<T>();
    }

    template <typename T>
    static const T& serializeString (const T& x)
    {
        return x;
    }

    template <typename T>
    static T deserializeString (const json& serial)
    {
        return serial.get<T>();
    }

private:
    JSONSerializer() = default; // static use only
};
} // namespace chowdsp
