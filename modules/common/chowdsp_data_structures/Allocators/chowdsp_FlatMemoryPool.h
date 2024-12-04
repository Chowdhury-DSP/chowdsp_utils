#pragma once

#if JUCE_WINDOWS
#else
#include <sys/mman.h>
#endif

namespace chowdsp
{
struct FlatMemoryPool
{
    std::byte* current_pointer {};
    std::byte* memory_base {};
    std::byte* first_uncommitted_page {};
    std::byte* address_limit {};
    size_t page_size {};

    static constexpr size_t DEFAULT_VIRTUAL_MEMORY_RESERVE = 256 * 1024 * 1024;

    void* allocate_bytes (size_t num_bytes, size_t alignment = 8)
    {
        auto* p = juce::snapPointerToAlignment (current_pointer, alignment);
        const auto end = p + num_bytes;

        if (end > first_uncommitted_page)
        {
            if (end > address_limit)
            {
                // The memory pool is full!
                jassertfalse;
                return nullptr;
            }
            else
            {
                extend_committed_pages (end);
            }
        }

        current_pointer = end;
        return p;
    }

    [[nodiscard]] size_t get_bytes_used() const noexcept
    {
        return static_cast<size_t> (current_pointer - memory_base);
    }

    void clear()
    {
        if (memory_base == nullptr)
            return;

#if ARENA_ALLOCATOR_DEBUG
        memset (memory_base, 0xcc, static_cast<size_t> (current_pointer - memory_base));
#endif

        current_pointer = memory_base;
    }

    struct Frame
    {
        FlatMemoryPool& pool;
        std::byte* pointer_at_start;

        explicit Frame (FlatMemoryPool& frame_pool)
            : pool { frame_pool },
              pointer_at_start { pool.current_pointer }
        {
        }

        ~Frame()
        {
            pool.current_pointer = pointer_at_start;
        }
    };

    /** Creates a frame for this allocator */
    auto create_frame()
    {
        return Frame { *this };
    }

    void init()
    {
        page_size = static_cast<size_t> (juce::SystemStats::getPageSize());
        const auto reserve_padded = page_size * ((DEFAULT_VIRTUAL_MEMORY_RESERVE + page_size - 1) / page_size);

#if JUCE_WINDOWS
#else
        memory_base = static_cast<std::byte*> (mmap (nullptr, reserve_padded, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0));
        jassert (memory_base != nullptr);
#endif

        current_pointer = memory_base;
        first_uncommitted_page = memory_base;
        address_limit = memory_base + reserve_padded;
    }

    void deinit()
    {
        if (memory_base == nullptr)
            return;

#if JUCE_WINDOWS
#else
        munmap (memory_base, static_cast<size_t> (address_limit - memory_base));
#endif

        memory_base = nullptr;
    }

private:
#if JUCE_WINDOWS
#else
    void extend_committed_pages (std::byte* end)
    {
        jassert (end - first_uncommitted_page >= 0);
        first_uncommitted_page = juce::snapPointerToAlignment (current_pointer, page_size);
    }
#endif
};
} // namespace chowdsp
