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
        // magic number to identify memory blocks that we've stored as XML
        const juce::uint32 magicXmlNumber = 0x21324356;
        serial->writeTo (outStream);
    }

    static SerializedType fromFile (const juce::File& file)
    {
        auto serial = juce::parseXML (file);
        jassert (serial != nullptr); // unable to load from file!
        return serial;
    }

    static SerializedType fromXML(const juce::XmlElement* xmlElement)
    {
        return std::unique_ptr<juce::XmlElement>(const_cast<juce::XmlElement*>(xmlElement));
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

    static auto createBaseElement(juce::String id)
    {
        return std::make_unique<juce::XmlElement> (id);
    }

    static void addChildElement (SerializedType& parent, SerializedType&& newChild) // NOSONAR
    {
        parent->addChildElement (newChild.release());
    }
    static void addChildElement (SerializedType& parent, juce::String newChild) // NOSONAR
    {
        parent->addChildElement (new juce::XmlElement(newChild));
    }

    static void addChildElement (SerializedType& parent, juce::String element, float f) // NOSONAR
    {
        parent->setAttribute (element, juce::String(f,3));
    }
    static void addChildElement (SerializedType& parent, juce::String element,juce::Point<int> point) // NOSONAR
    {
        parent->setAttribute ("point_x",juce::String(point.getX()));
        parent->setAttribute ("point_y",juce::String(point.getY()));

    }
    static void addChildElement (SerializedType& parent, juce::String element, juce::String f) // NOSONAR
    {
        parent->setAttribute (element, f);
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
    static juce::XmlElement* getChildElement (DeserializedType parent, juce::String attr )
    {
//        if (parent == nullptr || ! juce::isPositiveAndBelow (index, parent->getNumChildElements()))
//        {
//            jassertfalse;
//            return {};
//        }

        return parent->getChildByName(attr);
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

    static int getNumAttributes (DeserializedType serial)
    {
        if (serial == nullptr)
        {
            jassertfalse;
            return 0;
        }

        return serial->getNumAttributes();
    }
    static juce::String getAttributeName (DeserializedType serial, int i)
    {
        if (serial == nullptr)
        {
            jassertfalse;
            return "";
        }

        return serial->getAttributeName(i);
    }

    static juce::String getAttribute (DeserializedType serial, int i)
    {
        if (serial == nullptr)
        {
            jassertfalse;
            return "";
        }

        return serial->getAttributeName(i);
    }



    template <typename T>
    static std::enable_if_t<std::is_integral_v<T>, SerializedType> serializeArithmeticType (SerializedType& parent, juce::String id, T x)
    {

        parent->setAttribute (id, (int) x);
        return parent;
    }

    template <typename T>
    static std::enable_if_t<std::is_floating_point_v<T>, SerializedType> serializeArithmeticType (SerializedType& parent, juce::String id, T x)
    {
        parent->setAttribute (id, (double) x);
        return parent;
    }

    template <typename T>
    static std::enable_if_t<std::is_integral_v<T>, T>
        deserializeArithmeticType (DeserializedType serial, juce::String id)
    {
        if (serial == nullptr)
            return T {};

        return (T) serial->getIntAttribute (id);
    }

    template <typename T>
    static std::enable_if_t<std::is_floating_point_v<T>, T>
        deserializeArithmeticType (DeserializedType serial, juce::String id)
    {
        if (serial == nullptr)
            return T {};

        return (T) serial->getDoubleAttribute (id);
    }

    template <typename T>
    static SerializedType serializeEnumType (SerializedType& parent, juce::String id, T x)
    {
        return serializeArithmeticType (parent, id,static_cast<int> ( x));
    }

    template <typename T>
    static T deserializeEnumType (DeserializedType serial, juce::String id)
    {
        return static_cast<T> (deserializeArithmeticType<int> (serial, id));
    }

    template <typename T>
    static SerializedType serializeString (SerializedType& parent, juce::String id, const T& x)
    {
        parent->setAttribute (id, x);
        return std::move(parent);
    }

    template <typename T>
    static std::enable_if_t<std::is_same_v<T, std::string>, T>
        deserializeString (DeserializedType serial, juce::String id)
    {
        if (serial == nullptr)
            return T {};

        return deserializeString<juce::String> (serial,id).toStdString();
    }

    template <typename T>
    static std::enable_if_t<std::is_same_v<T, juce::String>, T>
        deserializeString (DeserializedType serial, juce::String id)
    {
        if (serial == nullptr)
            return T {};

        return serial->getStringAttribute (id);
    }

private:
    XMLSerializer() = default; // static use only

    static inline juce::Identifier defaultID = "ID";
};
} // namespace chowdsp
