#pragma once

#if JUCE_MODULE_AVAILABLE_chowdsp_reflection
#include <chowdsp_reflection/chowdsp_reflection.h>

namespace chowdsp
{
template <typename Key, typename T>
struct EnumMap
{
    static_assert (std::is_enum_v<Key>, "The key type must be an enum!");

    EnumMap() = default;

    constexpr EnumMap (std::initializer_list<std::pair<Key, T>> init)
    {
        for (auto& [key, value] : init)
            insert_or_assign (key, value);
    }

    EnumMap (const EnumMap&) = delete;
    EnumMap& operator= (const EnumMap&) = delete;

    EnumMap (EnumMap&& other) noexcept = delete;
    EnumMap& operator= (EnumMap&& other) noexcept = delete;

    ~EnumMap()
    {
        // TODO
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        for (auto& opt : storage)
        {
            if (opt.has_value())
                return false;
        }
        return true;
    }

    [[nodiscard]] constexpr size_t size() const noexcept
    {
        size_t count = 0;
        for (auto& opt : storage)
        {
            if (opt.has_value())
                count++;
        }
        return count;
    }

    [[nodiscard]] static constexpr size_t max_size() noexcept
    {
        return enum_count;
    }

    constexpr void insert_or_assign (Key key, T value)
    {
        storage[get_index (key)] = value;
    }

    template <typename... Args>
    constexpr T& emplace (Key key, Args&&... args)
    {
        auto& opt = storage[get_index (key)];
        opt.emplace (std::forward<Args> (args)...);
        return *opt;
    }

    [[nodiscard]] constexpr T& at (Key key)
    {
        // @TODO: what if this returns std::nullopt?
        return *storage[get_index (key)];
    }

    [[nodiscard]] constexpr const T& at (Key key) const
    {
        // @TODO: what if this returns std::nullopt?
        return *storage[get_index (key)];
    }

    [[nodiscard]] constexpr T& operator[] (Key key)
    {
        // @TODO: should we or should we not null-check here?
        return at (key);
    }

    [[nodiscard]] constexpr const T& operator[] (Key key) const
    {
        // @TODO: should we or should we not null-check here?
        return at (key);
    }

private:
    static constexpr auto enum_count = magic_enum::enum_count<Key>();
    std::array<std::optional<T>, enum_count> storage;

    constexpr size_t get_index (Key k) const
    {
        return *magic_enum::enum_index (k);
    }
};
} // namespace chowdsp

#endif // JUCE_MODULE_AVAILABLE_chowdsp_reflection
