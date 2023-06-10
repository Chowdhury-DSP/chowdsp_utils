#pragma once

#include <array>

namespace chowdsp
{
template <typename T>
struct RawObject
{
    template <typename... Args>
    T* construct (Args&&... args)
    {
        return new (data()) T (std::forward<Args> (args)...);
    }

    void destruct()
    {
        item().~T();
    }

    std::array<std::byte, sizeof (T)> raw_data;
    T* data() { return reinterpret_cast<T*> (raw_data.data()); } // NOSONAR
    const T* data() const { return reinterpret_cast<const T*> (raw_data.data()); } // NOSONAR
    T& item() { return *data(); }
    const T& item() const { return *data(); }
};
} // namespace chowdsp
