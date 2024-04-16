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

    ~ChainedArenaAllocator()
    {
        free_arenas();
    }

    /**
     * Selects the size used for each of the internal arena allocators,
     * and "resets" the allocator back to a single arena with that size.
     */
    void reset (size_t head_arena_size_bytes)
    {
        arena_size_bytes = head_arena_size_bytes;

        free_arenas();
        arenas.clear();

        arenas.emplace_front (arena_size_bytes);
        current_arena = arenas.begin();
        allocate_current_arena (arena_size_bytes);
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
        return get_current_arena().template data<T> (offset_bytes);
    }

    /** Returns the arena currently being used */
    ArenaAllocatorView& get_current_arena()
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

    /**
     * Returns the total number of bytes currently being used
     * by this allocator.
     *
     * Note that due to the design of the allocator, not all
     * of the available bytes may be used at any given time.
     * As such, the allocator's "load factor" can be computed
     * as: get_total_bytes_used() / get_total_bytes()
     */
    [[nodiscard]] size_t get_total_bytes_used() const noexcept
    {
        size_t bytes_count = 0;
        for (auto arena_iter = arenas.begin(); arena_iter != current_arena; ++arena_iter)
            bytes_count += arena_iter->get_bytes_used();
        bytes_count += current_arena->get_bytes_used();
        return bytes_count;
    }

    /**
     * Returns the total number of bytes being managed by
     * this allocator.
     */
    [[nodiscard]] size_t get_total_bytes() const noexcept
    {
        size_t bytes_count = 0;
        for (const auto& arena : arenas)
            bytes_count += arena.get_total_num_bytes();
        return bytes_count;
    }

    /** Merges another allocator into this one, and invalidates the other allocator. */
    void merge (ChainedArenaAllocator& allocator_to_merge)
    {
        if (allocator_to_merge.arena_count == 0)
            return; // no work to do!

        // both arenas must have the same head size!
        jassert (arena_size_bytes == 0 || arena_size_bytes == allocator_to_merge.arena_size_bytes);

        if (arena_count == 0)
        {
            *this = std::move (allocator_to_merge);
            return;
        }

        size_t new_arena_count = 0;
        auto merge_iter = allocator_to_merge.arenas.begin();
        for (; merge_iter != allocator_to_merge.current_arena; ++merge_iter)
        {
            arenas.push_front (std::move (*merge_iter));
            new_arena_count++;
        }

        jassert (merge_iter == allocator_to_merge.current_arena);
        arenas.push_front (std::move (*merge_iter));
        new_arena_count++;
        merge_iter++;

        for (; merge_iter != allocator_to_merge.arenas.end(); ++merge_iter)
        {
            arenas.insert_after (current_arena, std::move (*merge_iter));
            new_arena_count++;
        }

        arena_count += new_arena_count;

        allocator_to_merge = {};
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
        const std::forward_list<ArenaAllocatorView>::iterator arena_at_start;
        ArenaAllocatorView::Frame arena_frame;
    };

    /** Creates a frame for this allocator */
    auto create_frame()
    {
        return Frame { *this };
    }

private:
    void add_arena_to_chain()
    {
        const auto prev_arena = current_arena++;

        // if we've reached the end of the list, then we need to
        // add a new arena to the chain (starting after the previous arena)
        if (current_arena == arenas.end())
        {
            current_arena = arenas.emplace_after (prev_arena);
            allocate_current_arena (arena_size_bytes);
            arena_count++;
            return;
        }

        get_current_arena().clear();
    }

    void allocate_current_arena (size_t num_bytes)
    {
        static constexpr size_t arena_alignment = 64;
        const auto num_bytes_padded = arena_alignment * ((num_bytes + arena_alignment - 1) / arena_alignment);
        current_arena->get_memory_resource() = {
            static_cast<std::byte*> (aligned_alloc (arena_alignment, num_bytes_padded)),
            num_bytes_padded,
        };
    }

    void free_arenas()
    {
        for (auto& arena : arenas)
            aligned_free (arena.get_memory_resource().data());
    }

    std::forward_list<ArenaAllocatorView> arenas {};
    std::forward_list<ArenaAllocatorView>::iterator current_arena {};
    size_t arena_size_bytes = 0;
    size_t arena_count = 0;
};
} // namespace chowdsp
