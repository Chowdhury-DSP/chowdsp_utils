#pragma once

namespace chowdsp
{
/**
 * A memory arena designed to be used with juce::Component objects
 * (although other types can be allocated into the arena as well).
 */
template <size_t arena_chunk_size_bytes = 8192, size_t approx_component_count = 32>
class ComponentArena
{
public:
    ComponentArena() = default;
    ComponentArena (const ComponentArena&) = delete;
    ComponentArena& operator= (const ComponentArena&) = delete;
    ComponentArena (ComponentArena&&) noexcept = delete;
    ComponentArena& operator= (ComponentArena&&) noexcept = delete;

    ~ComponentArena()
    {
        clear_all();
    }

    /** Allocates a single object into the arena. */
    template <typename T, typename... Args>
    T* allocate (Args&&... args)
    {
        auto* bytes = allocator.allocate_bytes (sizeof (T), alignof (T));
        auto* new_component = new (bytes) T { std::forward<Args> (args)... };

        if constexpr (std::is_base_of_v<juce::Component, T>)
            component_list.emplace_back (new_component);

        return new_component;
    }

    /** Allocates multiple objects into the arena. */
    template <typename T, typename... Args>
    nonstd::span<T> allocate_n (size_t n, Args&&... args)
    {
        auto* bytes = allocator.allocate_bytes (sizeof (T) * n, alignof (T));
        auto span = nonstd::span<T> { reinterpret_cast<T*> (bytes), n };
        for (auto& ptr : span)
        {
            auto* new_component = new (&ptr) T { std::forward<Args> (args)... };
            if constexpr (std::is_base_of_v<juce::Component, T>)
                component_list.emplace_back (new_component);
        }
        return span;
    }

    /**
     * Reclaims the arena memory, and destroys any components
     * that have been allocated into the arena.
     */
    void clear_all()
    {
        component_list.clear();
        allocator.clear();
    }

    ChainedArenaAllocator allocator { arena_chunk_size_bytes };

private:
    SmallVector<DestructiblePointer<juce::Component>, approx_component_count> component_list {};
};
} // namespace chowdsp
