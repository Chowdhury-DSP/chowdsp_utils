#pragma once

namespace chowdsp
{
namespace Serialization
{
    template <typename Serializer, typename TypeToSerialize>
    typename Serializer::SerializedType serialize (const TypeToSerialize& objectToSerialize)
    {
        return Serializer::serialize (objectToSerialize);
    }
} // namespace Serialization
} // namespace chowdsp
