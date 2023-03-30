#pragma once

namespace chowdsp
{
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
template <typename T, size_t MaxSize>
class LocalPointer
{
public:
    /** The type being pointed to */
    using element_type = T;

    LocalPointer() = default;
    ~LocalPointer() { reset(); }

    LocalPointer (const LocalPointer&) = delete;
    LocalPointer& operator= (const LocalPointer&) = delete;
    LocalPointer (LocalPointer&&) noexcept = default;
    LocalPointer& operator= (LocalPointer&&) noexcept = default;

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
        reset();
        pointer = new (data.data()) C (std::forward<Args> (args)...);
        return reinterpret_cast<C*> (pointer);
    }

    /**
     * If an instance has been constructed it will be destroyed,
     * and the local storage will be zero-ed.
     */
    void reset()
    {
        if (pointer != nullptr)
        {
            // Why are we calling the destructor explicitly here?
            // For some types it won't matter since we're zero-ing
            // the local storage anyway, but if the type has something
            // like a std::vector or std::unique_ptr inside it, or has
            // some custom logic in its destructor, then we need to make
            // sure that stuff gets taken care of before destroying the object.
            pointer->~T();

            pointer = nullptr;
            std::fill (std::begin (data), std::end (data), std::byte{});
        }
    }

    [[nodiscard]] T* get() { return pointer; }
    [[nodiscard]] const T* get() const { return pointer; }

    [[nodiscard]] T* operator->() { return get(); }
    [[nodiscard]] const T* operator->() const { return get(); }
    [[nodiscard]] T& operator*() { return *get(); }
    [[nodiscard]] const T& operator*() const { return *get(); }

private:
    T* pointer = nullptr;
    std::array<std::byte, MaxSize> data {};
};

template <typename T, size_t N>
bool operator== (const LocalPointer<T, N>& p1, std::nullptr_t)
{
    return p1.get() == nullptr;
}

template <typename T, size_t N>
bool operator!= (const LocalPointer<T, N>& p1, std::nullptr_t)
{
    return p1.get() != nullptr;
}

template <typename T, size_t N>
bool operator== (std::nullptr_t, const LocalPointer<T, N>& p2)
{
    return nullptr == p2.get();
}

template <typename T, size_t N>
bool operator!= (std::nullptr_t, const LocalPointer<T, N>& p2)
{
    return nullptr != p2.get();
}
} // namespace chowdsp
