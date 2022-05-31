#pragma once

namespace chowdsp
{
/** Methods for serializing data */
namespace Serialization
{
    /** Serialize an object with a given serializer */
    template <typename Serializer, typename TypeToSerialize>
    typename Serializer::SerializedType serialize (const TypeToSerialize& objectToSerialize)
    {
        static_assert (std::is_base_of_v<BaseSerializer, Serializer>, "Serializer type must be derived from BaseSerializer");

        return Serializer::template serialize<Serializer> (objectToSerialize);
    }

    /** Serialize an object to a file with a given serializer */
    template <typename Serializer, typename TypeToSerialize>
    void serialize (const TypeToSerialize& objectToSerialize, const juce::File& targetFile)
    {
        Serializer::toFile (serialize<Serializer> (objectToSerialize), targetFile);
    }

    /** Deserialize an object with a given serializer */
    template <typename Serializer, typename TypeToDeserialize>
    void deserialize (const typename Serializer::SerializedType& serial, TypeToDeserialize& objectToDeserialize)
    {
        static_assert (std::is_base_of_v<BaseSerializer, Serializer>, "Serializer type must be derived from BaseSerializer");

        auto deserial = Serializer::template getDeserial<Serializer> (serial);
        return Serializer::template deserialize<Serializer> (deserial, objectToDeserialize);
    }

    /** Deserialize an object from a file with a given serializer */
    template <typename Serializer, typename TypeToDeserialize>
    void deserialize (const juce::File& file, TypeToDeserialize& objectToDeserialize)
    {
        deserialize<Serializer> (Serializer::fromFile (file), objectToDeserialize);
    }

    /** Deserialize an object from binary data with a given serializer */
    template <typename Serializer, typename TypeToDeserialize>
    void deserialize (const void* data, int dataSize, TypeToDeserialize& objectToDeserialize)
    {
        deserialize<Serializer> (Serializer::fromBinaryData (data, dataSize), objectToDeserialize);
    }
} // namespace Serialization
} // namespace chowdsp
