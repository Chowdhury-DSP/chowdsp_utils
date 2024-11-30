namespace chowdsp
{
nonstd::span<const std::byte> get_bytes_for_deserialization (nonstd::span<const std::byte>& bytes)
{
    using namespace bytes_detail;

    size_type bytes_count;
    std::memcpy (&bytes_count, bytes.data(), sizeof_s);
    jassert (sizeof_s + bytes_count <= bytes.size());

    const auto serial_bytes = bytes.subspan (sizeof_s, bytes_count);
    bytes = bytes.subspan (sizeof_s + bytes_count);

    return serial_bytes;
}

std::string_view deserialize_string (nonstd::span<const std::byte>& bytes)
{
    const auto serial_bytes = get_bytes_for_deserialization (bytes);
    return { reinterpret_cast<const char*> (serial_bytes.data()), serial_bytes.size() };
}
} // namespace chowdsp
