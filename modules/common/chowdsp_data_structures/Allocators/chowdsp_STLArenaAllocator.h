#pragma once

namespace chowdsp
{
/** A (mostly) STL-conforming allocator backed by a memory arena of your choosing. */
template <class T, typename ArenaType>
struct STLArenaAllocator
{
    using value_type = T;

    ArenaType& arena;

    explicit STLArenaAllocator (ArenaType& a) noexcept : arena (a)
    {
    }

    template <class U>
    STLArenaAllocator (const STLArenaAllocator<U, ArenaType>& other) noexcept : arena (other.arena) // NOLINT
    {
    }

    template <typename U>
    struct rebind
    {
        using other = STLArenaAllocator<U, ArenaType>;
    };

    T* allocate (std::size_t n)
    {
        return static_cast<T*> (arena.allocate_bytes (n, alignof (T)));
    }

    void deallocate (T*, std::size_t)
    {
        // no-op...
        // memory will be re-claimed when the arena is cleared.
    }
};

template <class T, class U, typename Arena>
constexpr bool operator== (const STLArenaAllocator<T, Arena>& x, const STLArenaAllocator<U, Arena>& y) noexcept
{
    return &x.arena == &y.arena;
}

template <class T, class U, typename Arena>
constexpr bool operator!= (const STLArenaAllocator<T, Arena>& x, const STLArenaAllocator<U, Arena>& y) noexcept
{
    return ! (x == y);
}
} // namespace chowdsp
