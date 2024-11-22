#pragma once

namespace chowdsp
{
/**
 * Linked lists are great... but the traditional way of implementing them
 * (i.e. one pointer per object) has a lot of memory overhead for small objects.
 *
 * This is a container that implements a linked list with the objects stored in
 * "chunks". The user can choose the chunk size to optimize for memory overhead
 * in your use-case.
 *
 * At the moment, this container only supports insertion, not removal.
 *
 * The list is backed by an arena allocator. If you're using a `chowdsp::ArenaAllocator`,
 * you must ensure that the arena is pre-reserved with enough space for everything in
 * the list. If you're using a `chowdsp::ChainedArenaAllocator` you should ensure that
 * the internal arena size is large enough for at least one "chunk" of the list.
 */
template <typename T, size_t chunk_size, typename ArenaType = ChainedArenaAllocator>
struct ChunkList
{
    static_assert (std::is_default_constructible_v<T>, "T must be default-constructible!");

    struct Chunk
    {
        std::array<T, chunk_size> chunk {};
        size_t count = 0;
        Chunk* next = nullptr;
    };

    // The list internals are exposed here in case you need to do anything fancy.
    // Please don't touch them unless you know what you are doing!
    ArenaType* arena {};
    Chunk head_chunk {};
    Chunk* tail_chunk = &head_chunk;

    /** Constructs a ChunkList with a backing arena. */
    explicit ChunkList (ArenaType& arena_to_use) : arena { &arena_to_use }
    {
    }

    ChunkList (ChunkList&&) noexcept = default;
    ChunkList& operator= (ChunkList&&) noexcept = default;

    /**
     * This will "reset" the ChunkList. If you actually want to reclaim the memory
     * being used by the ChunkList, that should be done at the arena level.
     */
    void clear()
    {
        head_chunk = {};
        tail_chunk = &head_chunk;
    }

    /** Inserts an object into the list (by move) */
    void insert (T&& next)
    {
        grow_if_needed();
        tail_chunk->chunk[tail_chunk->count++] = std::move (next);
    }

    /** Inserts an object into the list (by copy) */
    void insert (const T& next)
    {
        grow_if_needed();
        tail_chunk->chunk[tail_chunk->count++] = next;
    }

    /** For each (non-const) */
    template <typename Func>
    void for_each (Func&& func)
    {
        for (auto* chunk = &head_chunk; chunk != nullptr; chunk = chunk->next)
        {
            for (size_t i = 0; i < chunk->count; ++i)
                func (chunk->chunk[i]);
        }
    }

    /** For each (const) */
    template <typename Func>
    void for_each (Func&& func) const
    {
        for (auto* chunk = &head_chunk; chunk != nullptr; chunk = chunk->next)
        {
            for (size_t i = 0; i < chunk->count; ++i)
                func (chunk->chunk[i]);
        }
    }

    /** Returns the number of items currently in the list. */
    [[nodiscard]] size_t count() const
    {
        size_t count_result = 0;
        for (auto* chunk = &head_chunk; chunk != nullptr; chunk = chunk->next)
            count_result += chunk->count;
        return count_result;
    }

    template <bool is_const>
    struct iterator
    {
        std::conditional_t<is_const, const Chunk*, Chunk*> chunk {};
        size_t index_in_chunk {};
        bool operator!= (const iterator& other) const
        {
            return chunk != other.chunk || index_in_chunk != other.index_in_chunk;
        }
        void operator++()
        {
            ++index_in_chunk;
            if (index_in_chunk == chunk->count && chunk->next != nullptr)
            {
                chunk = chunk->next;
                index_in_chunk = 0;
            }
        }
        auto& operator*() const { return chunk->chunk[index_in_chunk]; }
    };
    [[nodiscard]] auto begin() { return iterator<false> { &head_chunk }; }
    [[nodiscard]] auto end() { return iterator<false> { tail_chunk, tail_chunk->count }; }
    [[nodiscard]] auto begin() const { return iterator<true> { &head_chunk }; }
    [[nodiscard]] auto end() const { return iterator<true> { tail_chunk, tail_chunk->count }; }

private:
    void grow_if_needed()
    {
#if CHOWDSP_HAS_CPP20
        if (tail_chunk->count == chunk_size) [[unlikely]]
#else
        if (tail_chunk->count == chunk_size)
#endif
        {
            auto* next_chunk_data = arena->allocate_bytes (sizeof (Chunk), alignof (Chunk));
            jassert (next_chunk_data != nullptr);
            tail_chunk->next = new (next_chunk_data) Chunk {};
            tail_chunk = tail_chunk->next;
        }
    }
};
} // namespace chowdsp
