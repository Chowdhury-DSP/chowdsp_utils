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

    /** Creates an owning optional pointer */
    template <typename... Args>
    explicit OptionalPointer (Args&&... args)
        : owningPtr (std::make_unique<T> (std::forward<Args> (args)...)),
          nonOwningPtr (owningPtr.get())
    {
    }

    /** Creates a non-owning optional pointer */
    explicit OptionalPointer (T* ptr)
        : owningPtr (nullptr),
          nonOwningPtr (ptr)
    {
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

template <class T>
struct IsOptionalPointerType : std::false_type
{
};

template <class T>
struct IsOptionalPointerType<OptionalPointer<T>> : std::true_type
{
};

/** True if the type is a chowdsp::OptionalPointer<T> */
template <typename T>
static constexpr bool IsOptionalPointer = IsOptionalPointerType<T>::value;
} // namespace chowdsp
