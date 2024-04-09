#pragma once

namespace chowdsp
{
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

    T* allocate (std::size_t n)
    {
        auto* ptr = static_cast<T*> (arena.allocate_bytes (n, alignof (T)));
        return ptr;
    }

    void deallocate (T*, std::size_t)
    {
        // no-op...
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
}
