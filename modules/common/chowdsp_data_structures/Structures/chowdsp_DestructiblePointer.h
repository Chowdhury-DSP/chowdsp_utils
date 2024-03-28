#pragma once

namespace chowdsp
{
/**
 * A wrapper for a pointer that will call the destructor
 * for its underlying object when it goes out of scope,
 * but WILL NOT free the underlying memory.
 *
 * This may be useful if the pointer is referring to some
 * stack memory, or if the memory will be freed elsewhere,
 * e.g. as part of a memory arena.
 */
template <typename T>
struct DestructiblePointer
{
    DestructiblePointer() = default;
    DestructiblePointer (T* ptr) : pointer (ptr) {} // NOLINT(google-explicit-constructor)
    DestructiblePointer (const DestructiblePointer&) = delete;
    DestructiblePointer& operator= (const DestructiblePointer&) = delete;
    DestructiblePointer (DestructiblePointer&& other) noexcept : pointer (other.pointer)
    {
        other.pointer = nullptr;
    }
    DestructiblePointer& operator= (DestructiblePointer&& other) noexcept
    {
        if (this != &other)
        {
            destroy();
            pointer = other.pointer;
            other.pointer = nullptr;
        }
        return *this;
    }

    ~DestructiblePointer()
    {
        destroy();
    }

    /**
     * Calls the object destructor, and resets the pointer to null.
     * Does nothing if the pointer is already null.
     */
    void destroy()
    {
        if (pointer != nullptr)
            pointer->~T();
        pointer = nullptr;
    }

    /** Releases the pointer without destroying. */
    T* release()
    {
        auto* ptr = pointer;
        pointer = nullptr;
        return pointer;
    }

    [[nodiscard]] T* get() { return pointer; }
    [[nodiscard]] const T* get() const { return pointer; }

    [[nodiscard]] T* operator->() { return get(); }
    [[nodiscard]] const T* operator->() const { return get(); }
    [[nodiscard]] T& operator*() { return *get(); }
    [[nodiscard]] const T& operator*() const { return *get(); }

private:
    T* pointer = nullptr;
};

template <typename T>
bool operator== (const DestructiblePointer<T>& p1, std::nullptr_t)
{
    return p1.get() == nullptr;
}

template <typename T>
bool operator!= (const DestructiblePointer<T>& p1, std::nullptr_t)
{
    return p1.get() != nullptr;
}

template <typename T>
bool operator== (std::nullptr_t, const DestructiblePointer<T>& p2)
{
    return nullptr == p2.get();
}

template <typename T>
bool operator!= (std::nullptr_t, const DestructiblePointer<T>& p2)
{
    return nullptr != p2.get();
}
} // namespace chowdsp
