#pragma once

namespace chowdsp
{
#ifndef DOXYGEN
namespace bytes_detail
{
    using size_type = uint64_t;
    static constexpr auto sizeof_s = sizeof (size_type);
} // namespace bytes_detail
#endif

template <typename ArenaType>
static std::byte* get_bytes_for_serialization (size_t bytes_count, ArenaType& arena)
{
    using namespace bytes_detail;
    jassert (bytes_count <= std::numeric_limits<size_type>::max());

    if constexpr (std::is_same_v<ArenaType, ChainedArenaAllocator>)
        jassert (sizeof_s + bytes_count <= arena.get_default_arena_size());

    auto* bytes = static_cast<std::byte*> (arena.allocate_bytes (sizeof_s + bytes_count, 1));
    jassert (bytes != nullptr);

    const auto bytes_count_cast = static_cast<size_type> (bytes_count);
    std::memcpy (bytes, &bytes_count_cast, sizeof_s);

    return bytes + sizeof_s; // NOLINT
}

nonstd::span<const std::byte> get_bytes_for_deserialization (nonstd::span<const std::byte>& bytes);

template <typename TDest, typename TSource>
void serialize_direct (TDest* ptr, const TSource& source)
{
    const auto source_cast = static_cast<TDest> (source);
    std::memcpy (ptr, &source_cast, sizeof (TDest));
}

template <typename T, typename ArenaType>
static size_t serialize_object (const T& object, ArenaType& arena)
{
    auto* bytes = get_bytes_for_serialization (sizeof (T), arena);
    std::memcpy (bytes, &object, sizeof (T)); // NOLINT
    return bytes_detail::sizeof_s + sizeof (T);
}

template <typename T, typename ArenaType>
static size_t serialize_span (nonstd::span<const T> data, ArenaType& arena)
{
    const auto num_bytes = sizeof (T) * data.size();
    auto* bytes = get_bytes_for_serialization (num_bytes, arena);
    std::memcpy (bytes, data.data(), num_bytes); // NOLINT
    return bytes_detail::sizeof_s + num_bytes;
}

template <typename ArenaType>
static size_t serialize_string (std::string_view str, ArenaType& arena)
{
    const auto num_bytes = sizeof (char) * str.size();
    auto* bytes = get_bytes_for_serialization (num_bytes, arena);
    std::memcpy (bytes, str.data(), num_bytes); // NOLINT
    return bytes_detail::sizeof_s + num_bytes;
}

template <typename MemoryResourceType>
static nonstd::span<const std::byte> dump_serialized_bytes (const ArenaAllocator<MemoryResourceType>& arena,
                                                            const typename ArenaAllocator<MemoryResourceType>::Frame* frame = nullptr)
{
    const auto bytes_offset = frame == nullptr ? 0 : frame->bytes_used_at_start;
    const auto bytes_count = arena.get_bytes_used() - bytes_offset;
    return { arena.template data<std::byte> (bytes_offset), bytes_count };
}

size_t get_serial_num_bytes (ChainedArenaAllocator& arena,
                             const ChainedArenaAllocator::Frame* frame = nullptr);

void dump_serialized_bytes (nonstd::span<std::byte> serial,
                            ChainedArenaAllocator& arena,
                            const ChainedArenaAllocator::Frame* frame = nullptr);

void dump_serialized_bytes (juce::MemoryBlock& data,
                            ChainedArenaAllocator& arena,
                            const ChainedArenaAllocator::Frame* frame = nullptr);

template <typename T>
T deserialize_direct (nonstd::span<const std::byte>& bytes)
{
    T x;
    std::memcpy (&x, bytes.data(), sizeof (T));
    bytes = bytes.subspan (sizeof (T));
    return x;
}

template <typename T>
static T deserialize_object (nonstd::span<const std::byte>& bytes)
{
    const auto serial_bytes = get_bytes_for_deserialization (bytes);
    jassert (serial_bytes.size() == sizeof (T));

    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wclass-memaccess")
    T object;
    std::memcpy (&object, serial_bytes.data(), serial_bytes.size());
    return object;
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
}

template <typename T>
static size_t get_span_length (const nonstd::span<const std::byte>& bytes)
{
    using namespace bytes_detail;

    size_type sizeof_span_bytes;
    std::memcpy (&sizeof_span_bytes, bytes.data(), sizeof_s);

    jassert (sizeof_span_bytes % sizeof (T) == 0);
    return static_cast<size_t> (sizeof_span_bytes) / sizeof (T);
}

template <typename T>
static void deserialize_span (nonstd::span<T> span, nonstd::span<const std::byte>& bytes)
{
    const auto serial_bytes = get_bytes_for_deserialization (bytes);

    jassert (serial_bytes.size() % sizeof (T) == 0);
    jassert (serial_bytes.size() / sizeof (T) == span.size());

    std::memcpy (span.data(), serial_bytes.data(), serial_bytes.size());
}

std::string_view deserialize_string (nonstd::span<const std::byte>& bytes);
} // namespace chowdsp
