#pragma once

namespace chowdsp::arena
{
/**
 * Allocates space for some number of objects of type T
 * The returned memory will be un-initialized, so be sure to clear it manually if needed.
 */
template <typename T, typename IntType, typename Arena>
T* allocate (Arena& arena, IntType num_Ts, size_t alignment = alignof (T)) noexcept
{
    return static_cast<T*> (arena.allocate_bytes ((size_t) num_Ts * sizeof (T), alignment));
}

template <typename T, typename I, typename Arena>
nonstd::span<T> make_span (Arena& arena, I size, size_t alignment = alignof (T))
{
    return { allocate<T> (arena, size, alignment), static_cast<size_t> (size) };
}

template <typename Arena>
std::string_view alloc_string (Arena& arena, const std::string_view& str)
{
    auto* data = arena.template allocate<char> (str.size());
    std::copy (str.begin(), str.end(), data);
    return std::string_view { data, str.size() }; // NOLINT
}

#if CHOWDSP_USING_JUCE
template <typename Arena>
std::string_view alloc_string (Arena& arena, const juce::String& str)
{
    return alloc_string (arena, toStringView (str));
}
#endif
} // namespace chowdsp::arena
