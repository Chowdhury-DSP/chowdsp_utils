#pragma once

#include <memory>

namespace chowdsp
{
/**
 * A pointer type that may or may not own the data it points to.
 *
 * Unlike std::unique_ptr, this pointer may release ownership while still
 * pointing to the same object. When releasing ownership, make sure that the
 * new owner has a longer lifetime than this object, or invalidates this object
 * before being deleted, otherwise this object will be a dangling pointer.
 */
template <typename T>
struct OptionalPointer
{
    /** The type being pointed to */
    using element_type = T;

    /** Initializes a null OptionalPointer */
    OptionalPointer() = default;

    /** Creates an optional pointer from a raw pointer, and optionally taking ownership */
    explicit OptionalPointer (T* ptr, bool shouldOwn = true)
        : owningPtr (shouldOwn ? ptr : nullptr),
          nonOwningPtr (shouldOwn ? owningPtr.get() : ptr)
    {
    }

    /** Creates an owning optional pointer */
    template <typename Arg1, typename... Args, typename = typename std::enable_if_t<! (std::is_pointer_v<Arg1> && std::is_base_of_v<T, std::remove_pointer_t<Arg1>>)>>
    explicit OptionalPointer (Arg1&& arg1, Args&&... args)
        : owningPtr (std::make_unique<T> (std::forward<Arg1> (arg1), std::forward<Args> (args)...)),
          nonOwningPtr (owningPtr.get())
    {
    }

    /**
     * Sets OptionalPointer to point to a new pointer, and take ownership of it.
     * If the OptionalPointer previously owned some data, that data will be deleted.
     */
    void setOwning (T* ptr)
    {
        owningPtr.reset (ptr);
        nonOwningPtr = ptr;
    }

    /**
     * Sets OptionalPointer to point to a new pointer, and take ownership of it.
     * If the OptionalPointer previously owned some data, that data will be deleted.
     */
    void setOwning (std::unique_ptr<T>&& ptr)
    {
        owningPtr = std::move (ptr);
        nonOwningPtr = owningPtr.get();
    }

    /**
     * Sets OptionalPointer to point to a new pointer, WITHOUT taking ownership of it.
     * If the OptionalPointer previously owned some data, that data will be deleted.
     */
    void setNonOwning (T* ptr)
    {
        owningPtr.reset();
        nonOwningPtr = ptr;
    }

    /** Move constructor */
    OptionalPointer (OptionalPointer&&) noexcept = default;

    /** Move assignment */
    OptionalPointer& operator= (OptionalPointer&&) noexcept = default;

    OptionalPointer (const OptionalPointer&) = delete;
    OptionalPointer& operator= (const OptionalPointer&) = delete;

    /** Returns true if this pointer owns the data it's pointing to */
    [[nodiscard]] bool isOwner() const noexcept { return owningPtr != nullptr; }

    /**
     * Releases ownership of the underlying data.
     *
     * Make sure the new owner has a longer lifetime than this
     * object, or calls `invalidate()` before being deleted.
     */
    [[nodiscard]] T* release()
    {
        jassert (isOwner()); // Pointer has already been released!
        return owningPtr.release();
    }

    /**
     * Resets this object ot nullptr. If this object currently
     * owns the underlying data, it will free the underlying data.
     */
    void invalidate()
    {
        if (isOwner())
            owningPtr.reset();
        nonOwningPtr = nullptr;
    }

    [[nodiscard]] T* get() { return nonOwningPtr; }
    [[nodiscard]] const T* get() const { return nonOwningPtr; }

    [[nodiscard]] operator T&() { return *nonOwningPtr; } // NOSONAR, NOLINT(google-explicit-constructor): we want to be able to do implicit conversion here
    [[nodiscard]] T* operator->() { return nonOwningPtr; }
    [[nodiscard]] const T* operator->() const { return nonOwningPtr; }
    [[nodiscard]] T& operator*() { return *nonOwningPtr; }
    [[nodiscard]] const T& operator*() const { return *nonOwningPtr; }

private:
    std::unique_ptr<T> owningPtr {};
    T* nonOwningPtr = nullptr;
};

template <typename T>
bool operator== (const OptionalPointer<T>& p1, const OptionalPointer<T>& p2)
{
    return p1.get() == p2.get();
}

template <typename T>
bool operator!= (const OptionalPointer<T>& p1, const OptionalPointer<T>& p2)
{
    return p1.get() != p2.get();
}

template <typename T>
bool operator== (const OptionalPointer<T>& p1, const std::unique_ptr<T>& p2) // NOSONAR
{
    return p1.get() == p2.get();
}

template <typename T>
bool operator!= (const OptionalPointer<T>& p1, const std::unique_ptr<T>& p2) // NOSONAR
{
    return p1.get() != p2.get();
}

template <typename T>
bool operator== (const std::unique_ptr<T>& p1, const OptionalPointer<T>& p2) // NOSONAR
{
    return p1.get() == p2.get();
}

template <typename T>
bool operator!= (const std::unique_ptr<T>& p1, const OptionalPointer<T>& p2) // NOSONAR
{
    return p1.get() != p2.get();
}

template <typename T>
bool operator== (const OptionalPointer<T>& p1, std::nullptr_t)
{
    return p1.get() == nullptr;
}

template <typename T>
bool operator!= (const OptionalPointer<T>& p1, std::nullptr_t)
{
    return p1.get() != nullptr;
}

template <typename T>
bool operator== (std::nullptr_t, const OptionalPointer<T>& p2)
{
    return nullptr == p2.get();
}

template <typename T>
bool operator!= (std::nullptr_t, const OptionalPointer<T>& p2)
{
    return nullptr != p2.get();
}

#ifndef DOXYGEN
template <class T>
struct IsOptionalPointerType : std::false_type
{
};

template <class T>
struct IsOptionalPointerType<OptionalPointer<T>> : std::true_type
{
};
#endif

/** True if the type is a chowdsp::OptionalPointer<T> */
template <typename T>
static constexpr bool IsOptionalPointer = IsOptionalPointerType<T>::value;
} // namespace chowdsp
