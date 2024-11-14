#pragma once

namespace chowdsp
{
/** A pointer that can be "packed" together with some flags. */
template <typename T>
struct PackedPointer
{
    static_assert (alignof (T) >= 8, "PackedPointer implementation requires pointers that are aligned to 8 bytes or greater!");

    static constexpr auto max_flag_value = alignof (T) - 1;
    static constexpr auto address_mask = ~static_cast<uintptr_t> (max_flag_value);
    static constexpr auto flags_mask = static_cast<uintptr_t> (max_flag_value);

    PackedPointer() = default;

    /** Initializes the pointer, optionally with some flags. */
    explicit PackedPointer (T* ptr, uint8_t flags = 0)
    {
        set (ptr, flags);
    }

    /** Modifies both the pointer and the flags. */
    void set (T* ptr, uint8_t flags = 0)
    {
        jassert (flags <= max_flag_value);
#if JUCE_DEBUG
        actual_ptr = ptr;
#endif
        ptr_with_flags = reinterpret_cast<T*> ((reinterpret_cast<uintptr_t> (ptr) & address_mask) // NOSONAR
                                               | (static_cast<uintptr_t> (flags) & flags_mask));
    }

    /** Modifies the flags without modifying the pointer. */
    void set_flags (uint8_t flags)
    {
        jassert (flags <= max_flag_value);
        ptr_with_flags = reinterpret_cast<T*> ((reinterpret_cast<uintptr_t> (ptr_with_flags) & address_mask) // NOSONAR
                                               | (static_cast<uintptr_t> (flags) & flags_mask));
    }

    /** Returns the pointer */
    [[nodiscard]] T* get_ptr()
    {
        return reinterpret_cast<T*> (reinterpret_cast<uintptr_t> (ptr_with_flags) & address_mask); // NOSONAR
    }

    /** Returns the pointer */
    [[nodiscard]] const T* get_ptr() const
    {
        return reinterpret_cast<T*> (reinterpret_cast<uintptr_t> (ptr_with_flags) & address_mask); // NOSONAR
    }

    /** Returns the flags */
    [[nodiscard]] uint8_t get_flags() const noexcept
    {
        return static_cast<uint8_t> (reinterpret_cast<uintptr_t> (ptr_with_flags) & flags_mask); // NOSONAR
    }

    [[nodiscard]] T* operator->() { return get_ptr(); }
    [[nodiscard]] const T* operator->() const { return get_ptr(); }
    [[nodiscard]] T& operator*() { return *get_ptr(); }
    [[nodiscard]] const T& operator*() const { return *get_ptr(); }

    void swap (PackedPointer& other) noexcept
    {
        std::swap (other.ptr_with_flags, ptr_with_flags);
#if JUCE_DEBUG
        std::swap (other.actual_ptr, actual_ptr);
#endif
    }

private:
    T* ptr_with_flags = nullptr;
#if JUCE_DEBUG
    [[maybe_unused]] T* actual_ptr = nullptr;
#endif
};

template <typename T>
bool operator== (const PackedPointer<T>& p1, T* other)
{
    return p1.get_ptr() == other;
}

template <typename T>
bool operator!= (const PackedPointer<T>& p1, T* other)
{
    return p1.get_ptr() != other;
}

template <typename T>
bool operator== (T* other, const PackedPointer<T>& p1)
{
    return p1.get_ptr() == other;
}

template <typename T>
bool operator!= (T* other, const PackedPointer<T>& p1)
{
    return p1.get_ptr() != other;
}

template <typename T>
bool operator== (const PackedPointer<T>& p1, std::nullptr_t)
{
    return p1.get_ptr() == nullptr;
}

template <typename T>
bool operator!= (const PackedPointer<T>& p1, std::nullptr_t)
{
    return p1.get_ptr() != nullptr;
}

template <typename T>
bool operator== (std::nullptr_t, const PackedPointer<T>& p2)
{
    return nullptr == p2.get_ptr();
}

template <typename T>
bool operator!= (std::nullptr_t, const PackedPointer<T>& p2)
{
    return nullptr != p2.get_ptr();
}
} // namespace chowdsp
