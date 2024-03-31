#pragma once

#include <cstdlib>

namespace chowdsp
{
#if defined(_MSC_VER)

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
    return std::aligned_alloc (alignment, size);
}

/** MSVC-compatible implementation of aligned_free */
inline void aligned_free (void* data)
{
    std::free (data);
}

#endif
} // namespace chowdsp
