#pragma once

namespace chowdsp
{
/**
 * A simple pool allocator.
 *
 * This implementation is based on GingerBill's blog post:
 * https://www.gingerbill.org/article/2019/02/16/memory-allocation-strategies-004/
 */
template <size_t chunk_size, size_t alignment>
struct PoolAllocator
{
private:
    struct FreeListNode
    {
        FreeListNode* next = nullptr;
    };
    FreeListNode* free_list_head = nullptr;

public:
    static constexpr size_t chunk_size_padded = ((chunk_size + alignment - 1) / alignment) * alignment;
    nonstd::span<std::byte> backing_buffer {};

    PoolAllocator() = default;

    /** Creates the allocator with some number of chunks pre-allocated. */
    explicit PoolAllocator (size_t num_chunks)
    {
        resize (num_chunks);
    }

    ~PoolAllocator()
    {
        if (backing_buffer.data() != nullptr)
            aligned_free (backing_buffer.data());
    }

    /** Re-allocates the allocator's backing buffer to fit some number of chunks. */
    void resize (size_t num_chunks)
    {
        aligned_free (backing_buffer.data());

        const auto allocator_bytes = chunk_size_padded * num_chunks;
        backing_buffer = { static_cast<std::byte*> (aligned_alloc (alignment, allocator_bytes)), allocator_bytes };
        free_all();
    }

    /** Resets the allocator free list. */
    void free_all()
    {
        size_t chunk_count = backing_buffer.size() / chunk_size_padded;
        // Set all chunks to be free
        for (size_t i = 0; i < chunk_count; ++i)
        {
            auto* ptr = backing_buffer.data() + i * chunk_size_padded;
            auto* node = reinterpret_cast<FreeListNode*> (ptr);

            // Push free node onto the free list
            node->next = free_list_head;
            free_list_head = node;
        }
    }

    /** Allocates a single chunk. */
    void* allocate_chunk()
    {
        // Get latest free node
        auto* node = free_list_head;

        if (node == nullptr)
        {
            jassertfalse;
            return nullptr;
        }

        // Pop free node
        free_list_head = free_list_head->next;

        // Zero memory by default
        return memset (node, 0, chunk_size_padded);
    }

    /** Allocates a chunk and constructs an object in place. */
    template <typename T, typename... Args>
    T* allocate (Args&&... args)
    {
        static_assert (sizeof (T) <= chunk_size_padded);
        static_assert (alignof (T) <= alignment);

        auto* bytes = allocate_chunk();
        return new (bytes) T { std::forward<Args> (args)... };
    }

    /** Frees a single chunk. */
    void free_chunk (void* ptr)
    {
        if (ptr == nullptr)
            return;

        void* start = backing_buffer.data();
        void* end = backing_buffer.data() + static_cast<int> (backing_buffer.size());
        if (ptr < start || ptr >= end)
        {
            // this pointer was not allocated from this data!
            jassertfalse;
            return;
        }

        // Push free node
        auto* node = static_cast<FreeListNode*> (ptr);
        node->next = free_list_head;
        free_list_head = node;
    }

    /** Calls an object's destructor and then frees the memory. */
    template <typename T>
    void free (T* ptr)
    {
        if (ptr == nullptr)
            return;

        void* start = backing_buffer.data();
        void* end = backing_buffer.data() + static_cast<int> (backing_buffer.size());
        if (ptr < start || ptr >= end)
        {
            // this pointer was not allocated from this data!
            jassertfalse;
            return;
        }

        ptr->~T();
        free_chunk (ptr);
    }
};

/** A pool allocator that is specialized for a single object type. */
template <typename T>
struct ObjectPool : PoolAllocator<sizeof (T), alignof (T)>
{
    using PoolAllocator<sizeof (T), alignof (T)>::PoolAllocator;

    /** Allocates and constructs an object in place. */
    template <typename... Args>
    T* allocate (Args&&... args)
    {
        return PoolAllocator<sizeof (T), alignof (T)>::template allocate<T> (std::forward<Args> (args)...);
    }
};
} // namespace chowdsp
