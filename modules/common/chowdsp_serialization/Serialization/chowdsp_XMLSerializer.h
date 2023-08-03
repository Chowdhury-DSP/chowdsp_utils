#pragma once

namespace chowdsp
{
/** Serializer which serializes data into an XML format */
class XMLSerializer : public BaseSerializer
{
public:
    using SerializedType = std::unique_ptr<juce::XmlElement>;
    using DeserializedType = const juce::XmlElement*;

    template <typename Serializer>
    static DeserializedType getDeserial (const SerializedType& serial) // NOSONAR
    {
        return serial.get();
    }

    static juce::String toString (const SerializedType& serial) // NOSONAR
    {
        return serial->toString();
    }

    static void toFile (const SerializedType& serial, const juce::File& file) // NOSONAR
    {
        serial->writeTo (file);
    }

    static void toMemoryBlock (const SerializedType& serial, juce::MemoryBlock& block) // NOSONAR
    {
        auto&& outStream = juce::MemoryOutputStream (block, false);
        serial->writeTo (outStream);
    }

    static SerializedType fromFile (const juce::File& file)
    {
        auto serial = juce::parseXML (file);
        jassert (serial != nullptr); // unable to load from file!
        return serial;
    }

    static SerializedType fromMemoryBlock (const juce::MemoryBlock& block)
    {
        auto serial = juce::parseXML (block.toString());
        jassert (serial != nullptr); // unable to load from file!
        return serial;
    }

    static SerializedType fromBinaryData (const void* data, int dataSize)
    {
        auto serial = juce::parseXML (juce::String::createStringFromData (data, dataSize));
        jassert (serial != nullptr); // unable to load from binary data!
        return serial;
    }

    static auto createBaseElement()
    {
        return std::make_unique<juce::XmlElement> (defaultID);
    }

    static void addChildElement (SerializedType& parent, SerializedType&& newChild) // NOSONAR
    {
        parent->addChildElement (newChild.release());
    }

    static juce::XmlElement* getChildElement (DeserializedType parent, int index)
    {
        if (parent == nullptr || ! juce::isPositiveAndBelow (index, parent->getNumChildElements()))
        {
            jassertfalse;
            return {};
        }

        return parent->getChildElement (index);
    }

    static int getNumChildElements (DeserializedType serial)
    {
        if (serial == nullptr)
        {
            jassertfalse;
            return 0;
        }

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
    static SerializedType serializeEnumType (T x)
    {
        return serializeArithmeticType (static_cast<int> (x));
    }

    template <typename T>
    static T deserializeEnumType (DeserializedType serial)
    {
        return static_cast<T> (deserializeArithmeticType<int> (serial));
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
