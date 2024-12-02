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

size_t get_serial_num_bytes (ChainedArenaAllocator& arena,
                             const ChainedArenaAllocator::Frame* frame)
{
    size_t num_bytes = 0;
    auto* start_arena = frame == nullptr ? arena.get_arenas().head : frame->arena_at_start;
    const auto add_bytes_count = [&num_bytes, start_arena, frame] (const ArenaAllocatorView& arena_node)
    {
        size_t bytes_offset = 0;
        if (start_arena == &arena_node && frame != nullptr)
            bytes_offset = frame->arena_frame.bytes_used_at_start;
        num_bytes += arena_node.get_bytes_used() - bytes_offset;
    };

    for (auto* arena_node = start_arena; arena_node != &arena.get_current_arena(); arena_node = arena_node->next)
        add_bytes_count (*arena_node);
    add_bytes_count (arena.get_current_arena());

    return num_bytes;
}

void dump_serialized_bytes (nonstd::span<std::byte> serial,
                            ChainedArenaAllocator& arena,
                            const ChainedArenaAllocator::Frame* frame)
{
    const auto num_bytes = serial.size();
    jassert (num_bytes == get_serial_num_bytes (arena, frame));

    auto* start_arena = frame == nullptr ? arena.get_arenas().head : frame->arena_at_start;
    size_t bytes_counter = 0;
    const auto copy_bytes = [num_bytes, start_arena, frame, &serial, &bytes_counter] (const ArenaAllocatorView& arena_node)
    {
        size_t bytes_offset = 0;
        if (start_arena == &arena_node && frame != nullptr)
            bytes_offset = frame->arena_frame.bytes_used_at_start;

        const auto bytes_to_copy = std::min (arena_node.get_bytes_used() - bytes_offset,
                                             num_bytes - bytes_counter);
        std::memcpy (serial.data() + bytes_counter, arena_node.data<std::byte> (bytes_offset), bytes_to_copy);

        bytes_counter += bytes_to_copy;
    };
    for (auto* arena_node = start_arena; arena_node != &arena.get_current_arena(); arena_node = arena_node->next)
        copy_bytes (*arena_node);
    copy_bytes (arena.get_current_arena());
}

void dump_serialized_bytes (juce::MemoryBlock& data,
                            ChainedArenaAllocator& arena,
                            const ChainedArenaAllocator::Frame* frame)
{
    const auto initial_size = data.getSize();
    const auto num_bytes = get_serial_num_bytes (arena, frame);
    data.setSize (initial_size + num_bytes);
    dump_serialized_bytes ({ static_cast<std::byte*> (data.getData()) + initial_size, num_bytes }, arena, frame);
}

std::string_view deserialize_string (nonstd::span<const std::byte>& bytes)
{
    const auto serial_bytes = get_bytes_for_deserialization (bytes);
    return { reinterpret_cast<const char*> (serial_bytes.data()), serial_bytes.size() };
}
} // namespace chowdsp
