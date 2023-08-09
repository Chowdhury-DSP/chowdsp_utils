#pragma once

#include <vector>

namespace chowdsp
{
/** A simple stack allocator */
class StackAllocator
{
public:
    StackAllocator() = default;

    StackAllocator (const StackAllocator&) = delete;
    StackAllocator& operator= (const StackAllocator&) = delete;

    StackAllocator (StackAllocator&&) noexcept = default;
    StackAllocator& operator= (StackAllocator&&) noexcept = default;

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
    void* allocate_bytes (size_t num_bytes) noexcept
    {
        if (bytes_used + num_bytes > raw_data.size())
            return nullptr;

        auto* pointer = raw_data.data() + bytes_used;
        bytes_used += num_bytes;
        return pointer;
    }

    /** Allocates space for some number of objects of type T */
    template <typename T, typename IntType>
    T* allocate (IntType num_Ts) noexcept
    {
        return static_cast<T*> (allocate_bytes ((size_t) num_Ts * sizeof (T)));
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
    struct StackAllocatorFrame
    {
        explicit StackAllocatorFrame (StackAllocator& allocator)
            : alloc (allocator),
              bytes_used_at_start (alloc.bytes_used)
        {
        }

        ~StackAllocatorFrame()
        {
            alloc.bytes_used = bytes_used_at_start;
        }

        StackAllocator& alloc;
        const size_t bytes_used_at_start;
    };

private:
    std::vector<std::byte> raw_data {};
    size_t bytes_used = 0;
};
} // namespace chowdsp
