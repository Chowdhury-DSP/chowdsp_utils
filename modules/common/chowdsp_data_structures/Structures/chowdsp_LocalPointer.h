#pragma once

namespace chowdsp
{
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4324) // structure was padded due to alignment specifier

/**
 * A generic "owning" pointer constructed using local storage.
 *
 * If this seems a little bit like std::optional, that's because it is!
 * In most cases, you should prefer std::optional over LocalPointer. However,
 * std::optional does not work with types that have been forward-declared,
 * or are abstract.
 *
 * Make sure that no other type (e.g. std::unique_ptr) tries to take ownership
 * of the pointer being stored here, since that will result in a double-delete.
 */
template <typename T, size_t MaxSize, size_t Alignment = raw_object_detail::get_alignment<T>()>
class LocalPointer
{
public:
    /** The type being pointed to */
    using element_type = T;

    LocalPointer() = default;
    ~LocalPointer() { reset(); }

    LocalPointer (const LocalPointer&) = delete;
    LocalPointer& operator= (const LocalPointer&) = delete;
    LocalPointer (LocalPointer&& other) noexcept
    {
        // move construction is only valid if the pointer is null!
        jassert (pointer == nullptr);
        jassert (other.pointer == nullptr);
        juce::ignoreUnused (other);
    }
    LocalPointer& operator= (LocalPointer&& other) noexcept
    {
        // move assignment is only valid if the pointer is null!
        jassert (pointer == nullptr);
        jassert (other.pointer == nullptr);
        juce::ignoreUnused (other);
        return *this;
    }

    /**
     * Constructs an instance using in place memory.
     *
     * If a value had previously been constructed, it will be destroyed
     * before the new instance is constructed.
     */
    template <typename C = T, typename... Args>
    C* emplace (Args&&... args)
    {
        static_assert (MaxSize >= sizeof (C), "Type is too large to fit into MaxSize bytes!");
        static_assert (Alignment % alignof (C) == 0, "Type alignments are incompatible!");

        reset();
        pointer = new (data.data()) C (std::forward<Args> (args)...);
        return reinterpret_cast<C*> (pointer.get());
    }

    /**
     * If an instance has been constructed it will be destroyed,
     * and the local storage will be zero-ed.
     */
    void reset()
    {
        if (pointer != nullptr)
        {
            pointer.destroy();
            std::fill (std::begin (data), std::end (data), std::byte {});
        }
    }

    [[nodiscard]] T* get() { return pointer.get(); }
    [[nodiscard]] const T* get() const { return pointer.get(); }

    [[nodiscard]] T* operator->() { return get(); }
    [[nodiscard]] const T* operator->() const { return get(); }
    [[nodiscard]] T& operator*() { return *get(); }
    [[nodiscard]] const T& operator*() const { return *get(); }

private:
    alignas (Alignment) std::array<std::byte, MaxSize> data {};
    DestructiblePointer<T> pointer = nullptr;
};

template <typename T, size_t N, size_t A>
bool operator== (const LocalPointer<T, N, A>& p1, std::nullptr_t)
{
    return p1.get() == nullptr;
}

template <typename T, size_t N, size_t A>
bool operator!= (const LocalPointer<T, N, A>& p1, std::nullptr_t)
{
    return p1.get() != nullptr;
}

template <typename T, size_t N, size_t A>
bool operator== (std::nullptr_t, const LocalPointer<T, N, A>& p2)
{
    return nullptr == p2.get();
}

template <typename T, size_t N, size_t A>
bool operator!= (std::nullptr_t, const LocalPointer<T, N, A>& p2)
{
    return nullptr != p2.get();
}

JUCE_END_IGNORE_WARNINGS_MSVC
} // namespace chowdsp
