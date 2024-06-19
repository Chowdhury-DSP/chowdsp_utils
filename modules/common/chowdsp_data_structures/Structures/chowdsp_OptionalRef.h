#pragma once

namespace chowdsp
{
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

    OptionalRef (std::nullopt_t)// NOLINT(google-explicit-constructor)
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
} // namespace chowdsp
