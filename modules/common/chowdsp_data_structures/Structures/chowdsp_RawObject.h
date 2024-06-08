#pragma once

#include <array>

JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4324) // structure was padded due to alignment specifier

namespace chowdsp
{
#ifndef DOXYGEN
namespace raw_object_detail
{
    template <typename T>
    constexpr size_t get_size()
    {
        if constexpr (is_complete_type_v<T>)
            return sizeof (T);
        return 0;
    }

    template <typename T>
    constexpr size_t get_alignment()
    {
        if constexpr (is_complete_type_v<T>)
            return alignof (T);
        return 8;
    }
} // namespace raw_object_detail
#endif

/**
 * A type that constructs an object out of raw storage.
 *
 * Note that this object does not manage the object lifetime
 * or provide any information about whether or not the object
 * is initialized, so if you need keep track of those kinds of
 * things, you'll need to do them manually.
 */
template <typename T,
          size_t size_bytes = raw_object_detail::get_size<T>(),
          size_t alignment_bytes = raw_object_detail::get_alignment<T>()>
struct RawObject
{
    RawObject() = default;

    RawObject (const RawObject&) = delete;
    RawObject& operator= (const RawObject&) = delete;

    RawObject (RawObject&& other) noexcept
    {
#if JUCE_DEBUG
        // move construction is only valid if the object is un-initialized!
        if constexpr (! std::is_trivially_destructible_v<T>)
            jassert (is_initialized == false);
        is_initialized = other.is_initialized;
        other.is_initialized = false;
#endif
        std::copy (other.raw_data.begin(), other.raw_data.end(), raw_data.begin());
    }

    RawObject& operator= (RawObject&& other) noexcept
    {
#if JUCE_DEBUG
        // move assignment is only valid if the object is un-initialized!
        if constexpr (! std::is_trivially_destructible_v<T>)
            jassert (is_initialized == false);
        is_initialized = other.is_initialized;
        other.is_initialized = false;
#endif
        std::copy (other.raw_data.begin(), other.raw_data.end(), raw_data.begin());
        return *this;
    }

#if JUCE_DEBUG
    ~RawObject()
    {
        // Please clean up after yourself!
        if constexpr (! std::is_trivially_destructible_v<T>)
            jassert (is_initialized == false);
    }
#endif

    template <typename C = T, typename... Args>
    C* construct (Args&&... args)
    {
#if JUCE_DEBUG
        if constexpr (! std::is_trivially_destructible_v<T>)
            jassert (is_initialized == false); // you tried to construct a new object without destroying the old one!
        is_initialized = true;
#endif
        return new (data()) C (std::forward<Args> (args)...);
    }

    void destruct()
    {
#if JUCE_DEBUG
        jassert (is_initialized == true); // you tried to destroy an object without creating one first!
        is_initialized = false;
#endif
        item().~T();
    }

    alignas (alignment_bytes) std::array<std::byte, size_bytes> raw_data {};
#if JUCE_DEBUG
    bool is_initialized = false;
#endif

    T* data()
    {
        return reinterpret_cast<T*> (raw_data.data());
    } // NOSONAR
    const T* data() const { return reinterpret_cast<const T*> (raw_data.data()); } // NOSONAR
    T& item() { return *data(); }
    const T& item() const { return *data(); }
};
} // namespace chowdsp

JUCE_END_IGNORE_WARNINGS_MSVC
