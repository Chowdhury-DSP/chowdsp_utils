#pragma once

#include <memory>

namespace chowdsp
{
/**
 * A pointer type that may or may not own the data it points to.
 *
 * Unlike std::unique_ptr, this pointer may release ownership while
 * still pointing to the same object.
 */
template <typename T>
struct SmartPointer
{
    /** The type being pointed to */
    using element_type = T;

    /** Creates an owning smart pointer */
    template <typename... Args>
    explicit SmartPointer (Args&&... args)
        : owningPtr (std::make_unique<T> (std::forward<Args> (args)...)),
          nonOwningPtr (owningPtr.get())
    {
    }

    /** Creates a non-owning smart pointer */
    explicit SmartPointer (T* ptr)
        : owningPtr (nullptr),
          nonOwningPtr (ptr)
    {
    }

    /** Move constructor */
    SmartPointer (SmartPointer&&) noexcept = default;

    /** Move assignment */
    SmartPointer& operator= (SmartPointer&&) noexcept = default;

    SmartPointer (const SmartPointer&) = delete;
    SmartPointer& operator= (const SmartPointer&) = delete;

    /** Returns true if this pointer owns the data it's pointing to */
    [[nodiscard]] bool isOwner() const noexcept { return owningPtr != nullptr; }

    /** Releases ownership of the underlying data */
    T* release()
    {
        jassert (owningPtr != nullptr); // Pointer has already been released!
        return owningPtr.release();
    }

    T* get() { return nonOwningPtr; }
    const T* get() const { return nonOwningPtr; }

    operator T&() { return *nonOwningPtr; } // NOSONAR, NOLINT(google-explicit-constructor): we want to be able to do implicit conversion here
    T* operator->() { return nonOwningPtr; }
    const T* operator->() const { return nonOwningPtr; }
    T& operator*() { return *nonOwningPtr; }
    const T& operator*() const { return *nonOwningPtr; }

private:
    std::unique_ptr<T> owningPtr {};
    T* nonOwningPtr = nullptr;
};

template <typename T>
bool operator== (const SmartPointer<T>& p1, const SmartPointer<T>& p2)
{
    return p1.get() == p2.get();
}

template <typename T>
bool operator!= (const SmartPointer<T>& p1, const SmartPointer<T>& p2)
{
    return p1.get() != p2.get();
}

template <typename T>
bool operator== (const SmartPointer<T>& p1, const std::unique_ptr<T>& p2)
{
    return p1.get() == p2.get();
}

template <typename T>
bool operator!= (const SmartPointer<T>& p1, const std::unique_ptr<T>& p2)
{
    return p1.get() != p2.get();
}

template <typename T>
bool operator== (const std::unique_ptr<T>& p1, const SmartPointer<T>& p2)
{
    return p1.get() == p2.get();
}

template <typename T>
bool operator!= (const std::unique_ptr<T>& p1, const SmartPointer<T>& p2)
{
    return p1.get() != p2.get();
}

template <typename T>
bool operator== (const SmartPointer<T>& p1, std::nullptr_t)
{
    return p1.get() == nullptr;
}

template <typename T>
bool operator!= (const SmartPointer<T>& p1, std::nullptr_t)
{
    return p1.get() != nullptr;
}

template <typename T>
bool operator== (std::nullptr_t, const SmartPointer<T>& p2)
{
    return nullptr == p2.get();
}

template <typename T>
bool operator!= (std::nullptr_t, const SmartPointer<T>& p2)
{
    return nullptr != p2.get();
}

template <class T>
struct IsSmartPointerType : std::false_type
{
};

template <class T>
struct IsSmartPointerType<SmartPointer<T>> : std::true_type
{
};

/** True if the type is a chowdsp::SmartPointer<T> */
template <typename T>
static constexpr bool IsSmartPointer = IsSmartPointerType<T>::value;
} // namespace chowdsp
