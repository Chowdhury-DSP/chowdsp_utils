#pragma once

namespace chowdsp
{
class XMLSerializer : public BaseSerializer
{
public:
    using SerializedType = std::unique_ptr<juce::XmlElement>;
    using DeserializedType = const juce::XmlElement*;

    template <typename Serializer>
    static DeserializedType getDeserial (const SerializedType& serial)
    {
        return serial.get();
    }

    static juce::String toString (const SerializedType& serial)
    {
        return serial->toString();
    }

    static void toFile (const SerializedType& serial, const juce::File& file)
    {
        serial->writeTo (file);
    }

    static SerializedType fromFile (const juce::File& file)
    {
        auto serial = juce::parseXML (file);
        jassert (serial != nullptr);
        return serial;
    }

    static SerializedType fromBinaryData (const void* data, int dataSize)
    {
        auto serial = juce::parseXML (juce::String::createStringFromData (data, dataSize));
        jassert (serial != nullptr);
        return serial;
    }

    static auto createBaseElement()
    {
        return std::make_unique<juce::XmlElement> (defaultID);
    }

    static void addChildElement (SerializedType& parent, SerializedType&& newChild)
    {
        parent->addChildElement (newChild.release());
    }

    static auto getChildElement (DeserializedType parent, int index)
    {
        return parent->getChildElement (index);
    }

    static int getNumChildElements (DeserializedType serial)
    {
        if (serial == nullptr)
            return 0;

        return serial->getNumChildElements();
    }

    template <typename T>
    static std::enable_if_t<std::is_integral_v<T>, SerializedType> serializeArithmeticType (T x)
    {
        auto element = createBaseElement();
        element->setAttribute (defaultID, (int) x);
        return element;
    }

    template <typename T>
    static std::enable_if_t<std::is_floating_point_v<T>, SerializedType> serializeArithmeticType (T x)
    {
        auto element = createBaseElement();
        element->setAttribute (defaultID, (double) x);
        return element;
    }

    template <typename T>
    static std::enable_if_t<std::is_integral_v<T>, T>
        deserializeArithmeticType (DeserializedType serial)
    {
        if (serial == nullptr)
            return T {};

        return (T) serial->getIntAttribute (defaultID);
    }

    template <typename T>
    static std::enable_if_t<std::is_floating_point_v<T>, T>
        deserializeArithmeticType (DeserializedType serial)
    {
        if (serial == nullptr)
            return T {};

        return (T) serial->getDoubleAttribute (defaultID);
    }

    template <typename T>
    static SerializedType serializeString (const T& x)
    {
        auto element = createBaseElement();
        element->setAttribute (defaultID, x);
        return element;
    }

    template <typename T>
    static std::enable_if_t<std::is_same_v<T, std::string>, T>
        deserializeString (DeserializedType serial)
    {
        if (serial == nullptr)
            return T {};

        return deserializeString<juce::String> (serial).toStdString();
    }

    template <typename T>
    static std::enable_if_t<std::is_same_v<T, juce::String>, T>
        deserializeString (DeserializedType serial)
    {
        if (serial == nullptr)
            return T {};

        return serial->getStringAttribute (defaultID);
    }

private:
    XMLSerializer() = default; // static use only

    static inline juce::Identifier defaultID = "ID";
};
} // namespace chowdsp
