#pragma once

#include <vector>

namespace chowdsp
{
/** A simple arena allocator */
class ArenaAllocator
{
public:
    ArenaAllocator() = default;

    /** Constructs the arena with an initial allocated size. */
    explicit ArenaAllocator (size_t size_in_bytes)
    {
        reset (size_in_bytes);
    }

    ArenaAllocator (const ArenaAllocator&) = delete;
    ArenaAllocator& operator= (const ArenaAllocator&) = delete;

    ArenaAllocator (ArenaAllocator&&) noexcept = default;
    ArenaAllocator& operator= (ArenaAllocator&&) noexcept = default;

    /** Re-allocates the internal buffer with a given number of bytes */
    void reset (size_t new_size_bytes)
    {
        clear();
        raw_data.resize (new_size_bytes, {});
    }

    /** Resets the allocator */
    void clear() noexcept
    {
        bytes_used = 0;
    }

    /** Returns the number of bytes currently being used */
    [[nodiscard]] size_t get_bytes_used() const noexcept { return bytes_used; }

    /** Allocates a given number of bytes */
    void* allocate_bytes (size_t num_bytes, size_t alignment = 1) noexcept
    {
        auto* pointer = juce::snapPointerToAlignment (raw_data.data() + bytes_used, alignment);
        const auto bytes_increment = static_cast<size_t> (std::distance (raw_data.data() + bytes_used, pointer + num_bytes));

        if (bytes_used + bytes_increment > raw_data.size())
            return nullptr;

        bytes_used += bytes_increment;
        return pointer;
    }

    /** Allocates space for some number of objects of type T */
    template <typename T, typename IntType>
    T* allocate (IntType num_Ts, size_t alignment = alignof (T)) noexcept
    {
        return static_cast<T*> (allocate_bytes ((size_t) num_Ts * sizeof (T), alignment));
    }

    /** Returns a pointer to the internal buffer with a given offset in bytes */
    template <typename T, typename IntType>
    T* data (IntType offset_bytes) noexcept
    {
        return reinterpret_cast<T*> (raw_data.data() + offset_bytes);
    }

    /**
     * Creates a "frame" for the allocator.
     * Once the frame goes out of scope, the allocator will be reset
     * to whatever it's state was at the beginning of the frame.
     */
    struct ArenaAllocatorFrame
    {
        explicit ArenaAllocatorFrame (ArenaAllocator& allocator)
            : alloc (allocator),
              bytes_used_at_start (alloc.bytes_used)
        {
        }

        ~ArenaAllocatorFrame()
        {
            alloc.bytes_used = bytes_used_at_start;
        }

        ArenaAllocator& alloc;
        const size_t bytes_used_at_start;
    };

private:
    std::vector<std::byte> raw_data {};
    size_t bytes_used = 0;
};
} // namespace chowdsp
