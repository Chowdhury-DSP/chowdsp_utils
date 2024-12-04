#pragma once

#if ! JUCE_TEENSY

#if JUCE_WINDOWS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

namespace chowdsp
{
/**
 * A "flat" memory pool that reserves a ton of virtual memory,
 * and then commits pages of that memory as needed.
 */
struct FlatMemoryPool
{
    std::byte* current_pointer {};
    std::byte* memory_base {};
    std::byte* first_uncommitted_page {};
    std::byte* address_limit {};
    const size_t page_size { get_page_size() };

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

    void init (size_t reserve_bytes = 1 << 28)
    {
        const auto reserve_padded = page_size * ((reserve_bytes + page_size - 1) / page_size);

#if JUCE_WINDOWS
        memory_base = static_cast<std::byte*> (VirtualAlloc (nullptr, reserve_padded, MEM_RESERVE, PAGE_READWRITE));
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
        VirtualFree (memory_base, 0, MEM_RELEASE);
#else
        munmap (memory_base, static_cast<size_t> (address_limit - memory_base));
#endif

        memory_base = nullptr;
    }

private:
    void extend_committed_pages (std::byte* end)
    {
        jassert (end - first_uncommitted_page >= 0);
#if JUCE_WINDOWS
        const auto size = juce::snapPointerToAlignment (end, page_size) - first_uncommitted_page;
        VirtualAlloc (first_uncommitted_page, static_cast<size_t> (size), MEM_COMMIT, PAGE_READWRITE);
        first_uncommitted_page += size;
#else
        first_uncommitted_page = juce::snapPointerToAlignment (end, page_size);
#endif
    }

#if JUCE_WINDOWS
    static size_t get_page_size()
    {
        SYSTEM_INFO systemInfo;
        GetNativeSystemInfo (&systemInfo);
        return static_cast<size_t> (systemInfo.dwPageSize);
    }
#else
    static size_t get_page_size()
    {
        return static_cast<size_t> (sysconf (_SC_PAGESIZE));
    }
#endif
};
} // namespace chowdsp

#endif
