#pragma once

#include <forward_list>

namespace chowdsp
{
class ChainedArenaAllocator
{
public:
    ChainedArenaAllocator() = default;

    /** Constructs the arena with an initial allocated size. */
    explicit ChainedArenaAllocator (size_t size_in_bytes)
    {
        reset (size_in_bytes);
    }

    ChainedArenaAllocator (const ChainedArenaAllocator&) = delete;
    ChainedArenaAllocator& operator= (const ChainedArenaAllocator&) = delete;

    ChainedArenaAllocator (ChainedArenaAllocator&&) noexcept = default;
    ChainedArenaAllocator& operator= (ChainedArenaAllocator&&) noexcept = default;

    /**
     * Selects the size used for each of the internal arena allocators,
     * and "resets" the allocator back to a single arena with that size.
     */
    void reset (size_t head_arena_size_bytes)
    {
        arena_size_bytes = head_arena_size_bytes;

        arenas.clear();

        arenas.emplace_front (arena_size_bytes);
        current_arena = arenas.begin();
        arena_count = 1;
    }

    /**
     * Moves the allocator "stack pointer" back to zero,
     * effectively "reclaiming" all allocated memory.
     */
    void clear() noexcept
    {
        current_arena = arenas.begin();
        get_current_arena().clear();
    }

    /** Allocates a given number of bytes */
    void* allocate_bytes (size_t num_bytes, size_t alignment = 1) noexcept
    {
        if (num_bytes > arena_size_bytes)
        {
            jassertfalse;
            return nullptr;
        }

        auto pointer = get_current_arena().allocate_bytes (num_bytes, alignment);
        if (pointer != nullptr)
            return pointer;

        add_arena_to_chain();
        return allocate_bytes (num_bytes, alignment);
    }

    /** Allocates space for some number of objects of type T */
    template <typename T, typename IntType>
    T* allocate (IntType num_Ts, size_t alignment = alignof (T)) noexcept
    {
        return static_cast<T*> (allocate_bytes ((size_t) num_Ts * sizeof (T), alignment));
    }

    /** Returns a pointer to the head allocator with a given offset in bytes */
    template <typename T, typename IntType>
    T* data (IntType offset_bytes) noexcept
    {
        return get_current_arena().data<T> (offset_bytes);
    }

    /** Returns the arena currently being used */
    ArenaAllocator<>& get_current_arena()
    {
        jassert (current_arena != arenas.end());
        return *current_arena;
    }

    /** Returns a list of the allocator's internal arenas */
    [[nodiscard]] auto& get_arenas() const noexcept
    {
        return arenas;
    }

    /** Returns the number of arenas currently allocated */
    [[nodiscard]] size_t get_arena_count() const noexcept
    {
        return arena_count;
    }

    struct Frame
    {
        explicit Frame (ChainedArenaAllocator& allocator)
            : alloc (allocator),
              arena_at_start (alloc.current_arena),
              arena_frame (alloc.get_current_arena())
        {
        }

        ~Frame()
        {
            alloc.current_arena = arena_at_start;
        }

        ChainedArenaAllocator& alloc;
        const std::forward_list<ArenaAllocator<>>::iterator arena_at_start;
        ArenaAllocator<>::Frame arena_frame;
    };

    /** Creates a frame for this allocator */
    auto create_frame()
    {
        return Frame { *this };
    }

private:
    void add_arena_to_chain()
    {
        const auto prev_arena = current_arena;
        current_arena++;

        // if we've reached the end of the list, then we need to
        // add a new arena to the chain (starting after the previous arena)
        if (current_arena == arenas.end())
        {
            current_arena = arenas.emplace_after (prev_arena, arena_size_bytes);
            arena_count++;
            return;
        }

        get_current_arena().clear();
    }

    std::forward_list<ArenaAllocator<>> arenas {};
    std::forward_list<ArenaAllocator<>>::iterator current_arena {};
    size_t arena_size_bytes = 0;
    size_t arena_count = 0;
};
} // namespace chowdsp
