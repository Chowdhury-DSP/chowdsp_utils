#pragma once

#include <cstdlib>

namespace chowdsp
{
#if JUCE_TEENSY

/** MSVC-compatible implementation of aligned_alloc */
[[nodiscard]] inline void* aligned_alloc ([[maybe_unused]] size_t alignment, size_t size)
{
    // @TODO
    return malloc (size + alignment);
}

/** MSVC-compatible implementation of aligned_free */
inline void aligned_free (void* data)
{
    free (data);
}

#elif defined(_MSC_VER)

/** MSVC-compatible implementation of aligned_alloc */
[[nodiscard]] inline void* aligned_alloc (size_t alignment, size_t size)
{
    return _aligned_malloc (size, alignment);
}

/** MSVC-compatible implementation of aligned_free */
inline void aligned_free (void* data)
{
    _aligned_free (data);
}

#else

/** MSVC-compatible implementation of aligned_alloc */
[[nodiscard]] inline void* aligned_alloc (size_t alignment, size_t size)
{
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunguarded-availability-new"
#endif

    // size is required to be a multiple of alignment!
    const auto size_padded = ((size + alignment - 1) / alignment) * alignment;
    return std::aligned_alloc (alignment, size_padded);

#ifdef __clang__
#pragma clang diagnostic pop
#endif
}

/** MSVC-compatible implementation of aligned_free */
inline void aligned_free (void* data)
{
    std::free (data);
}

#endif
} // namespace chowdsp
