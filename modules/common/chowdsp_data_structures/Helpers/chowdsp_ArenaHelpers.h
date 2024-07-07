#pragma once

namespace chowdsp::arena
{
template <typename T, typename I, typename Arena>
nonstd::span<T> make_span (Arena& arena, I size, size_t alignment = alignof (T))
{
    return { arena.template allocate<T> (size, alignment), static_cast<size_t> (size) };
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
