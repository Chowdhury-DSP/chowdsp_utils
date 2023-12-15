#pragma once

#if JUCE_MODULE_AVAILABLE_juce_graphics
#include <juce_graphics/juce_graphics.h>
#endif

namespace chowdsp
{
#ifndef DOXYGEN
namespace serialization_detail
{
    struct DummySerializer
    {
        using SerializedType = json;
        using DeserializedType = const json&;

        static auto createBaseElement() { return SerializedType {}; }
        static void addChildElement (SerializedType&, SerializedType&&) {} //NOSONAR
        static auto getChildElement (DeserializedType, int) { return false; }
        static int getNumChildElements (DeserializedType) { return 0; }

        template <typename, typename C>
        static SerializedType serialize (const C&)
        {
            return {};
        }

        template <typename>
        static DeserializedType getDeserial (const SerializedType& serial)
        {
            return serial;
        }

        template <typename, typename C>
        static void deserialize (DeserializedType, C&)
        {
        }
    };

    // The reason we need this "detail" namespace is so that we can define these static method
    // checkers for determining if a class has custom methods for serialization/deserialization.
    CHOWDSP_CHECK_HAS_STATIC_METHOD (HasCustomSerializer, template serialize<DummySerializer>)
    CHOWDSP_CHECK_HAS_STATIC_METHOD (HasCustomDeserializer, template deserialize<DummySerializer>)
} // namespace serialization_detail
#endif

/**
 * Base class for Serializer objects. If you would like to create
 * a serializer for a particular serialization format, the new serializer
 * should be derived from BaseSerializer.
 */
class BaseSerializer
{
    template <typename Serializer>
    using SerialType = typename Serializer::SerializedType;

    template <typename Serializer>
    using DeserialType = typename Serializer::DeserializedType;

    template <typename T>
    static constexpr auto IsString = std::is_same_v<T, std::string> || std::is_same_v<T, juce::String>;

#if JUCE_MODULE_AVAILABLE_juce_graphics
    template <typename T>
    static constexpr auto IsPoint = is_specialization_of_v<T, juce::Point>;
#endif

    template <typename T>
    static constexpr auto IsContainerNotMapOrString = TypeTraits::IsIterable<T> && ! IsString<T> && ! TypeTraits::IsMapLike<T>;

    template <typename T>
    static constexpr auto IsNotDirectlySerializable = ! std::is_arithmetic_v<T> && ! std::is_enum_v<T> && ! IsString<T> && ! TypeTraits::IsIterable<T> && ! IsPoint<T>;

    template <typename T>
    static constexpr auto HasCustomSerialization = serialization_detail::HasCustomSerializer<T>;

    template <typename T>
    static constexpr auto HasCustomDeserialization = serialization_detail::HasCustomDeserializer<T>;

public:
    /** Converts a SerialType object into its corresponding DeserialType */
    template <typename Serializer>
    static DeserialType<Serializer> getDeserial (const SerialType<Serializer>& serial)
    {
        return serial;
    }

    /** Serializer for serial type */
    template <typename Serializer, typename T>
    static std::enable_if_t<std::is_same_v<T, SerialType<Serializer>>, SerialType<Serializer>>
        serialize (T x)
    {
        return x;
    }

    /** Deserializer for serial types */
    template <typename Serializer, typename T>
    static std::enable_if_t<std::is_same_v<T, SerialType<Serializer>>, void>
        deserialize (DeserialType<Serializer> serial, T& x)
    {
        x = serial;
    }

    /** Serializer for arithmetic types */
    template <typename Serializer, typename T>
    static std::enable_if_t<std::is_arithmetic_v<T>, SerialType<Serializer>>
        serialize (T x)
    {
        return Serializer::serializeArithmeticType (x);
    }

    /** Deserializer for arithmetic types */
    template <typename Serializer, typename T>
    static std::enable_if_t<std::is_arithmetic_v<T>, void>
        deserialize (DeserialType<Serializer> serial, T& x)
    {
        x = Serializer::template deserializeArithmeticType<T> (serial);
    }

    /** Serializer for enum types */
    template <typename Serializer, typename T>
    static std::enable_if_t<std::is_enum_v<T>, SerialType<Serializer>>
        serialize (T x)
    {
        return Serializer::serializeEnumType (x);
    }

    /** Deserializer for enum types */
    template <typename Serializer, typename T>
    static std::enable_if_t<std::is_enum_v<T>, void>
        deserialize (DeserialType<Serializer> serial, T& x)
    {
        x = Serializer::template deserializeEnumType<T> (serial);
    }

    /** Serializer for string types */
    template <typename Serializer, typename T>
    static std::enable_if_t<IsString<T>, SerialType<Serializer>>
        serialize (const T& x)
    {
        return Serializer::serializeString (x);
    }

    /** Deserializer for string types */
    template <typename Serializer, typename T>
    static std::enable_if_t<IsString<T>, void>
        deserialize (DeserialType<Serializer> serial, T& x)
    {
        x = Serializer::template deserializeString<T> (serial);
    }

#if JUCE_MODULE_AVAILABLE_juce_graphics
    /** Serializer for juce::Point types */
    template <typename Serializer, typename T>
    static std::enable_if_t<IsPoint<T>, SerialType<Serializer>>
        serialize (const T& point)
    {
        auto serial = Serializer::createBaseElement();
        Serializer::addChildElement (serial, serialize<Serializer> (point.x));
        Serializer::addChildElement (serial, serialize<Serializer> (point.y));
        return serial;
    }

    /** Deserializer for juce::Point types */
    template <typename Serializer, typename T>
    static std::enable_if_t<IsPoint<T>, void>
        deserialize (DeserialType<Serializer> serial, T& point)
    {
        if (Serializer::getNumChildElements (serial) != 2)
        {
            jassertfalse; // the serialized data does not contain the correct number of elements to fill this array!
            point = {};
            return;
        }

        deserialize<Serializer> (Serializer::getChildElement (serial, 0), point.x);
        deserialize<Serializer> (Serializer::getChildElement (serial, 1), point.y);
    }
#endif

    /** Serializer for container types */
    template <typename Serializer, typename T>
    static std::enable_if_t<IsContainerNotMapOrString<T> && ! std::is_same_v<T, SerialType<Serializer>>, SerialType<Serializer>>
        serialize (const T& container)
    {
        auto serial = Serializer::createBaseElement();
        for (const auto& val : container)
            Serializer::addChildElement (serial, serialize<Serializer> (val));

        return serial;
    }

    /** Serializer for map-like container types */
    template <typename Serializer, typename T>
    static std::enable_if_t<TypeTraits::IsMapLike<T>, SerialType<Serializer>>
        serialize (const T& container)
    {
        auto serial = Serializer::createBaseElement();
        for (const auto& [key, val] : container)
        {
            Serializer::addChildElement (serial, serialize<Serializer> (key));
            Serializer::addChildElement (serial, serialize<Serializer> (val));
        }

        return serial;
    }

    /** Deserializer for std::array */
    template <typename Serializer, typename T, size_t N>
    static void deserialize (DeserialType<Serializer> serial, std::array<T, N>& container)
    {
        if ((int) container.size() != Serializer::getNumChildElements (serial))
        {
            jassertfalse; // the serialized data does not contain the correct number of elements to fill this array!
            std::fill (container.begin(), container.end(), T {});
            return;
        }

        int serialIndex = 0;
        for (auto& val : container)
            deserialize<Serializer> (Serializer::getChildElement (serial, serialIndex++), val);
    }

    /** Deserializer for std::vector */
    template <typename Serializer, typename T>
    static void deserialize (DeserialType<Serializer> serial, std::vector<T>& container)
    {
        container.resize ((size_t) Serializer::getNumChildElements (serial));

        int serialIndex = 0;
        for (auto& val : container)
            deserialize<Serializer> (Serializer::getChildElement (serial, serialIndex++), val);
    }

    /** Deserializer for map-like containers */
    template <typename Serializer, typename T>
    static std::enable_if_t<TypeTraits::IsMapLike<T>, void>
        deserialize (DeserialType<Serializer> serial, T& container)
    {
        const auto numKeysAndValues = Serializer::getNumChildElements (serial);
        if (numKeysAndValues % 2 != 0)
        {
            jassertfalse; // mak-like data structure was not properly serialized!
            return;
        }

        container.clear();
        for (int keyIndex = 0; keyIndex < numKeysAndValues; keyIndex += 2)
        {
            typename T::key_type key {};
            deserialize<Serializer> (Serializer::getChildElement (serial, keyIndex), key);

            const auto valIndex = keyIndex + 1;
            typename T::mapped_type value {};
            deserialize<Serializer> (Serializer::getChildElement (serial, valIndex), value);

            container.emplace (std::make_pair (key, value));
        }
    }

    /** Serializer for generic aggregate types */
    template <typename Serializer, typename T>
    static std::enable_if_t<IsNotDirectlySerializable<T> && ! HasCustomSerialization<T>, SerialType<Serializer>> serialize (const T& object)
    {
        auto serial = Serializer::createBaseElement();
        pfr::for_each_field (object, [&serial] (const auto& field)
                             { Serializer::addChildElement (serial, serialize<Serializer> (field)); });

        return serial;
    }

    /** Deserializer for generic aggregate types */
    template <typename Serializer, typename T>
    static std::enable_if_t<IsNotDirectlySerializable<T> && ! HasCustomDeserialization<T>, void> deserialize (DeserialType<Serializer> serial, T& object)
    {
        int serialIndex = 0;
        pfr::for_each_field (object, [&serial, &serialIndex] (auto& field)
                             { deserialize<Serializer> (Serializer::getChildElement (serial, serialIndex++), field); });
    }

    /** Serializer for types with custom serialization behaviour */
    template <typename Serializer, typename T>
    static std::enable_if_t<HasCustomSerialization<T>, SerialType<Serializer>> serialize (const T& object)
    {
        return T::template serialize<Serializer> (object);
    }

    /** Deserializer for types with custom deserialization behaviour */
    template <typename Serializer, typename T>
    static std::enable_if_t<HasCustomDeserialization<T>, void> deserialize (DeserialType<Serializer> serial, T& object)
    {
        T::template deserialize<Serializer> (serial, object);
    }

protected:
    BaseSerializer() = default; // static use only
};
} // namespace chowdsp
