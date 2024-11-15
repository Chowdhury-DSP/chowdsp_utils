#pragma once

#include <forward_list>

namespace chowdsp
{
class ChainedArenaAllocator
{
    struct ArenaNode;
    struct ArenaList;

public:
    ChainedArenaAllocator() = default;

    /** Constructs the arena with an initial allocated size. */
    explicit ChainedArenaAllocator (size_t size_in_bytes)
    {
        reset (size_in_bytes);
    }

    ChainedArenaAllocator (const ChainedArenaAllocator&) = delete;
    ChainedArenaAllocator& operator= (const ChainedArenaAllocator&) = delete;

    ChainedArenaAllocator (ChainedArenaAllocator&& other) noexcept
    {
        *this = std::move (other);
    }

    ChainedArenaAllocator& operator= (ChainedArenaAllocator&& other) noexcept
    {
        arena_list.head = other.arena_list.head;
        arena_list.count = other.arena_list.count;
        current_arena = other.current_arena;
        arena_size_bytes = other.arena_size_bytes;
        extra_alloc_list = other.extra_alloc_list;

        other.arena_list = {};
        other.current_arena = nullptr;
        other.arena_size_bytes = 0;
        other.extra_alloc_list = nullptr;

        return *this;
    }

    ~ChainedArenaAllocator()
    {
        free_extra_allocs();
        free_arenas();
    }

    /**
     * Selects the size used for each of the internal arena allocators,
     * and "resets" the allocator back to a single arena with that size.
     */
    void reset (size_t head_arena_size_bytes)
    {
        arena_size_bytes = std::max (head_arena_size_bytes, (size_t) 32);

        free_extra_allocs();
        free_arenas();

        current_arena = bootstrap_arena (arena_size_bytes);
        arena_list.head = current_arena;
        arena_list.count = 1;
    }

    /**
     * Moves the allocator "stack pointer" back to zero,
     * effectively "reclaiming" all allocated memory.
     */
    void clear() noexcept
    {
        free_extra_allocs();
        current_arena = arena_list.head;
        get_current_arena().clear();
    }

    /** Allocates a given number of bytes */
    void* allocate_bytes (size_t num_bytes, size_t alignment = 1) noexcept
    {
        if (num_bytes > arena_size_bytes)
        {
            auto* extra_alloc = allocate<ExtraAlloc> (1);
            extra_alloc->ptr = aligned_alloc (std::max (alignment, (size_t) 8), num_bytes);
            extra_alloc->size = num_bytes;
            if (extra_alloc_list != nullptr)
                extra_alloc->next = extra_alloc_list;
            else
                extra_alloc->next = nullptr;
            extra_alloc_list = extra_alloc;

            return extra_alloc->ptr;
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
        jassert (current_arena != nullptr);
        return *current_arena;
    }

    /** Returns a list of the allocator's internal arenas */
    [[nodiscard]] auto& get_arenas() const noexcept
    {
        return arena_list;
    }

    /** Returns the number of arenas currently allocated */
    [[nodiscard]] size_t get_arena_count() const noexcept
    {
        return arena_list.count;
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
        for (auto* arena = arena_list.head; arena != current_arena; arena = arena->next)
            bytes_count += arena->get_bytes_used();
        bytes_count += current_arena->get_bytes_used();

        for (auto* extra_alloc = extra_alloc_list; extra_alloc != nullptr; extra_alloc = extra_alloc->next)
            bytes_count += extra_alloc->size;

        return bytes_count;
    }

    /**
     * Returns the total number of bytes being managed by
     * this allocator.
     */
    [[nodiscard]] size_t get_total_bytes() const noexcept
    {
        size_t bytes_count = 0;
        for (auto* arena = arena_list.head; arena != nullptr; arena = arena->next)
            bytes_count += arena->get_total_num_bytes();
        for (auto* extra_alloc = extra_alloc_list; extra_alloc != nullptr; extra_alloc = extra_alloc->next)
            bytes_count += extra_alloc->size;
        return bytes_count;
    }

    /** Merges another allocator into this one, and invalidates the other allocator. */
    void merge (ChainedArenaAllocator& allocator_to_merge)
    {
        if (allocator_to_merge.arena_list.count == 0 && allocator_to_merge.extra_alloc_list == nullptr)
            return; // no work to do!

        // both arenas must have the same head size!
        jassert (arena_size_bytes == 0 || arena_size_bytes == allocator_to_merge.arena_size_bytes);

        // if our arena is empty, just make this arena into the other arena!
        if (arena_list.count == 0 && extra_alloc_list == nullptr)
        {
            current_arena = allocator_to_merge.current_arena;
            arena_list = allocator_to_merge.arena_list;
            extra_alloc_list = allocator_to_merge.extra_alloc_list;
            allocator_to_merge.current_arena = nullptr;
            allocator_to_merge.arena_list = {};
            allocator_to_merge.extra_alloc_list = nullptr;
            return;
        }

        const auto arena_add_count = allocator_to_merge.arena_list.count;
        if (auto* after_current = allocator_to_merge.current_arena->next; after_current != nullptr)
        {
            // take all arenas after allocator_to_merge.current_arena,
            // and put them after out current_arena
            after_current->next = current_arena->next;
            current_arena->next = after_current;
        }

        if (allocator_to_merge.current_arena != allocator_to_merge.arena_list.head)
        {
            // take all of the arenas up to allocator_to_merge.current_arena,
            // and put them at the head of our list.
            allocator_to_merge.current_arena->next = arena_list.head;
            arena_list.head = allocator_to_merge.arena_list.head;
        }

        arena_list.count += arena_add_count;

        if (allocator_to_merge.extra_alloc_list != nullptr)
        {
            ExtraAlloc* end_of_list = allocator_to_merge.extra_alloc_list;
            while (end_of_list->next != nullptr)
                end_of_list = end_of_list->next;
            end_of_list->next = extra_alloc_list;
            extra_alloc_list = allocator_to_merge.extra_alloc_list;
        }

        allocator_to_merge.current_arena = nullptr;
        allocator_to_merge.arena_list = {};
        allocator_to_merge.extra_alloc_list = nullptr;
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
        ArenaNode* arena_at_start;
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
        // if we've reached the end of the list, then we need to
        // add a new arena to the chain (starting after the previous arena)
        if (current_arena->next == nullptr)
        {
            current_arena->next = bootstrap_arena (arena_size_bytes);
            current_arena = current_arena->next;
            arena_list.count++;
            return;
        }

        current_arena = current_arena->next;
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

    void free_arenas() const
    {
        for (auto* arena = arena_list.head; arena != nullptr;)
        {
            // we need to update the arena iterator before free-ing the data,
            // otherwise we'll lose the `next` pointer!
            auto* data_to_free = arena->raw_data_start;
            arena = arena->next;
            aligned_free (data_to_free);
        }
    }

    void free_extra_allocs()
    {
        for (auto* extra_alloc = extra_alloc_list; extra_alloc != nullptr; extra_alloc = extra_alloc->next)
            aligned_free (extra_alloc->ptr);
        extra_alloc_list = nullptr;
    }

    static ArenaNode* bootstrap_arena (size_t num_bytes)
    {
        static constexpr size_t arena_alignment = 64;

        num_bytes += sizeof (ArenaNode);
        const auto num_bytes_padded = arena_alignment * ((num_bytes + arena_alignment - 1) / arena_alignment);
        auto* data = static_cast<std::byte*> (aligned_alloc (arena_alignment, num_bytes_padded));

        auto* arena_node = new (data) ArenaNode {};
        arena_node->raw_data_start = data;
        arena_node->get_memory_resource() = {
            data + sizeof (ArenaNode),
            num_bytes_padded - sizeof (ArenaNode),
        };
        return arena_node;
    }

    struct ArenaNode : ArenaAllocatorView
    {
        std::byte* raw_data_start = nullptr;
        ArenaNode* next = nullptr;
    };

    struct ArenaList
    {
        ArenaNode* head = nullptr;
        size_t count = 0;
    };

    ArenaList arena_list {};
    ArenaNode* current_arena {};
    size_t arena_size_bytes = 0;

    struct ExtraAlloc
    {
        void* ptr = nullptr;
        size_t size {};
        ExtraAlloc* next = nullptr;
    };
    ExtraAlloc* extra_alloc_list = nullptr;
};
} // namespace chowdsp
