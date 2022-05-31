#pragma once

namespace chowdsp
{
/** Methods for serializing dataa */
namespace Serialization
{
    /** Serialize an object with a given serializer */
    template <typename Serializer, typename TypeToSerialize>
    typename Serializer::SerializedType serialize (const TypeToSerialize& objectToSerialize)
    {
        static_assert (std::is_base_of_v<BaseSerializer, Serializer>, "Serializer type must be derived from BaseSerializer");

        return Serializer::template serialize<Serializer> (objectToSerialize);
    }

    /** Deserialize an object with a given serializer */
    template <typename Serializer, typename TypeToDeserialize>
    void deserialize (const typename Serializer::SerializedType& serial, TypeToDeserialize& objectToDeserialize)
    {
        static_assert (std::is_base_of_v<BaseSerializer, Serializer>, "Serializer type must be derived from BaseSerializer");

        auto deserial = Serializer::template getDeserial<Serializer> (serial);
        return Serializer::template deserialize<Serializer> (deserial, objectToDeserialize);
    }
} // namespace Serialization
} // namespace chowdsp
