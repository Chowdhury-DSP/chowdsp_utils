#pragma once

namespace chowdsp
{
/**
 * This class is basically an implementation of std::optional<T&>.
 * Under the hood, it's basically just a pointer but with an
 * optional-like interface.
 *
 * Make sure that an OptionalRef never holds on to a reference
 * past the lifetime of the referenced object.
 */
template <typename T>
class OptionalRef
{
    T* ptr = nullptr;

public:
    OptionalRef() = default;
    OptionalRef (const OptionalRef&) = default;
    OptionalRef& operator= (const OptionalRef&) = default;
    OptionalRef (OptionalRef&&) noexcept = default;
    OptionalRef& operator= (OptionalRef&&) noexcept = default;

    OptionalRef (std::nullopt_t) // NOLINT(google-explicit-constructor)
    {
    }

    OptionalRef (T& value) // NOLINT(google-explicit-constructor)
        : ptr { &value }
    {
    }

    OptionalRef& operator= (std::nullopt_t) noexcept // NOLINT
    {
        ptr = nullptr;
        return *this;
    }

    OptionalRef& operator= (T& value) noexcept // NOLINT
    {
        ptr = &value;
        return *this;
    }

    T* operator->() noexcept { return ptr; }
    const T* operator->() const noexcept { return ptr; }
    T& operator*() noexcept { return *ptr; }
    const T& operator*() const noexcept { return *ptr; }

    [[nodiscard]] explicit operator bool() const noexcept { return has_value(); }
    [[nodiscard]] bool has_value() const noexcept { return ptr != nullptr; }

    T& value()
    {
        if (! has_value())
            throw std::bad_optional_access();
        return *ptr;
    }

    const T& value() const
    {
        if (! has_value())
            throw std::bad_optional_access();
        return *ptr;
    }

    void reset() noexcept { ptr = nullptr; }
    void swap (OptionalRef& other) noexcept { std::swap (ptr, other.ptr); }

    const T& value_or (const T& other) const { return has_value() ? *ptr : other; }
};

template <typename T>
bool operator== (const OptionalRef<T>& p1, const OptionalRef<T>& p2)
{
    if (p1.has_value() != p2.has_value())
        return false;

    if (! p1.has_value())
        return true; // both nullopt

    return *p1 == *p2;
}

template <typename T>
bool operator!= (const OptionalRef<T>& p1, const OptionalRef<T>& p2)
{
    return ! (p1 == p2);
}

template <typename T>
bool operator== (const OptionalRef<T>& p1, std::nullopt_t)
{
    return ! p1.has_value();
}

template <typename T>
bool operator!= (const OptionalRef<T>& p1, std::nullopt_t)
{
    return p1.has_value();
}

template <typename T>
bool operator== (std::nullopt_t, const OptionalRef<T>& p1)
{
    return ! p1.has_value();
}

template <typename T>
bool operator!= (std::nullopt_t, const OptionalRef<T>& p1)
{
    return p1.has_value();
}

template <typename T>
bool operator== (const OptionalRef<T>& p1, const std::remove_cv_t<T>& p2)
{
    if (! p1.has_value())
        return false;
    return *p1 == p2;
}

template <typename T>
bool operator!= (const OptionalRef<T>& p1, const std::remove_cv_t<T>& p2)
{
    return ! (p1 == p2);
}

template <typename T>
bool operator== (const std::remove_cv_t<T>& p2, const OptionalRef<T>& p1)
{
    return p1 == p2;
}

template <typename T>
bool operator!= (const std::remove_cv_t<T>& p2, const OptionalRef<T>& p1)
{
    return p1 != p2;
}
} // namespace chowdsp
