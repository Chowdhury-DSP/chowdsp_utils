#pragma once

#include <vector>

#ifndef ARENA_ALLOCATOR_DEBUG
#define ARENA_ALLOCATOR_DEBUG JUCE_DEBUG
#endif

namespace chowdsp
{
/** A simple arena allocator */
template <typename MemoryResourceType = std::vector<std::byte>>
class ArenaAllocator
{
public:
    ArenaAllocator() = default;

    /** Constructs the arena with an initial allocated size. */
    explicit ArenaAllocator (size_t size_in_bytes)
    {
        reset (size_in_bytes);
    }

    template <typename OtherMemoryResourceType,
              typename ThisMemoryResourceType = MemoryResourceType,
              std::enable_if_t<std::is_same_v<ThisMemoryResourceType, nonstd::span<std::byte>>>* = nullptr>
    ArenaAllocator (ArenaAllocator<OtherMemoryResourceType>& other) // NOLINT
        : raw_data { other.raw_data },
          bytes_used { other.bytes_used }
    {
    }

    ArenaAllocator (const ArenaAllocator&) = delete;
    ArenaAllocator& operator= (const ArenaAllocator&) = delete;

    ArenaAllocator (ArenaAllocator&&) noexcept = default;
    ArenaAllocator& operator= (ArenaAllocator&&) noexcept = default;

    /** Re-allocates the internal buffer with a given number of bytes */
    void reset (size_t new_size_bytes)
    {
        clear();

        if constexpr (MemoryResourceTypeHasResize<MemoryResourceType>)
            raw_data.resize (new_size_bytes, {});
    }

    /**
     * Moves the allocator "stack pointer" back to zero,
     * effectively "reclaiming" all allocated memory.
     */
    void clear() noexcept
    {
#if ARENA_ALLOCATOR_DEBUG
        std::fill (raw_data.begin(), raw_data.begin() + static_cast<int> (bytes_used), std::byte { 0xDD });
        std::fill (raw_data.begin() + static_cast<int> (bytes_used), raw_data.end(), std::byte { 0x00 });
#endif
        bytes_used = 0;
    }

    /** Returns the number of bytes currently being used. */
    [[nodiscard]] size_t get_bytes_used() const noexcept { return bytes_used; }

    /** Returns the total number of bytes the allocator manages. */
    [[nodiscard]] size_t get_total_num_bytes() const noexcept { return raw_data.size(); }

    /** Returns a reference to the allocator's internal memory resource. */
    [[nodiscard]] auto& get_memory_resource() { return raw_data; }

    /** Returns a reference to the allocator's internal memory resource. */
    [[nodiscard]] const auto& get_memory_resource() const { return raw_data; }

    /**
     * Allocates a given number of bytes.
     * The returned memory will be un-initialized, so be sure to clear it manually if needed.
     */
    void* allocate_bytes (size_t num_bytes, size_t alignment = 1) noexcept
    {
        auto* pointer = juce::snapPointerToAlignment (raw_data.data() + bytes_used, alignment);
        const auto bytes_increment = static_cast<size_t> (std::distance (raw_data.data() + bytes_used, pointer + num_bytes));

        if (bytes_used + bytes_increment > raw_data.size())
            return nullptr;

        bytes_used += bytes_increment;
        return pointer;
    }

    /**
     * Allocates space for some number of objects of type T
     * The returned memory will be un-initialized, so be sure to clear it manually if needed.
     */
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
    struct Frame
    {
        explicit Frame (ArenaAllocator& allocator)
            : alloc (allocator),
              bytes_used_at_start (alloc.bytes_used)
        {
        }

        ~Frame()
        {
            alloc.bytes_used = bytes_used_at_start;
        }

        ArenaAllocator& alloc;
        const size_t bytes_used_at_start;
    };

    /** Creates a frame for this allocator */
    auto create_frame()
    {
        return Frame { *this };
    }

private:
    MemoryResourceType raw_data {};
    size_t bytes_used_internal = 0;
    size_t& bytes_used = bytes_used_internal;

    template <typename>
    friend class ArenaAllocator;

    CHOWDSP_CHECK_HAS_METHOD (MemoryResourceTypeHasResize, resize, std::declval<size_t>())
};

/** Convenient alias for a "non-owning" arena. */
using ArenaAllocatorView = ArenaAllocator<nonstd::span<std::byte>>;
} // namespace chowdsp
