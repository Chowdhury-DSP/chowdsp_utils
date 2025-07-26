#pragma once

#if JUCE_MODULE_AVAILABLE_chowdsp_reflection
#include <chowdsp_reflection/chowdsp_reflection.h>

namespace chowdsp
{
/**
 * Similar to EnumMap, except that every enum value
 * is guaranteed to have a mapped value.
 */
template <typename Key, typename T>
struct EnumMapComplete
{
    static_assert (std::is_enum_v<Key>, "The key type must be an enum!");

    constexpr EnumMapComplete() = default;

    constexpr explicit EnumMapComplete (T default_value)
    {
        std::fill (storage.begin(), storage.end(), default_value);
    }

    constexpr EnumMapComplete (std::initializer_list<std::pair<Key, T>> init, T default_value = {})
        : EnumMapComplete { default_value }
    {
        for (auto& [key, value] : init)
            assign (key, value);
    }

    constexpr EnumMapComplete (const EnumMapComplete&) = delete;
    constexpr EnumMapComplete& operator= (const EnumMapComplete&) = delete;

    constexpr EnumMapComplete (EnumMapComplete&& other) noexcept = default;
    constexpr EnumMapComplete& operator= (EnumMapComplete&& other) noexcept = default;

    [[nodiscard]] static constexpr size_t size() noexcept
    {
        return enum_count;
    }

    constexpr void assign (Key key, T value) noexcept
    {
        storage[get_index (key)] = value;
    }

    [[nodiscard]] constexpr T& at (Key key)
    {
        return storage[get_index (key)];
    }

    [[nodiscard]] constexpr const T& at (Key key) const
    {
        return storage[get_index (key)];
    }

    [[nodiscard]] constexpr T& operator[] (Key key) noexcept
    {
        return at (key);
    }

    [[nodiscard]] constexpr const T& operator[] (Key key) const noexcept
    {
        return at (key);
    }

    template <typename Storage, bool is_const = false>
    struct iterator
    {
        size_t index;
        std::conditional_t<is_const, typename Storage::const_iterator, typename Storage::iterator> iter;
        Key key;

        bool operator!= (const iterator& other) const
        {
            return iter != other.iter;
        }

        void operator++()
        {
            ++index;
            ++iter;

            if (index < std::tuple_size_v<Storage>)
                key = magic_enum::enum_value<Key> (index);
        }

        auto operator*() const
        {
            return std::tie (key, *iter);
        }
    };

    constexpr auto begin() noexcept
    {
        return iterator<decltype (storage)> {
            0,
            storage.begin(),
            magic_enum::enum_value<Key> (0),
        };
    }

    constexpr auto begin() const noexcept
    {
        return iterator<decltype (storage), true> {
            0,
            storage.cbegin(),
            magic_enum::enum_value<Key> (0),
        };
    }

    constexpr auto end() noexcept
    {
        return iterator<decltype (storage)> {
            enum_count,
            storage.end(),
            {},
        };
    }

    constexpr auto end() const noexcept
    {
        return iterator<decltype (storage), true> {
            enum_count,
            storage.cend(),
            {},
        };
    }

private:
    static constexpr auto enum_count = magic_enum::enum_count<Key>();
    std::array<T, enum_count> storage {};

    constexpr size_t get_index (Key k) const
    {
        return *magic_enum::enum_index (k);
    }
};
} // namespace chowdsp

#endif // JUCE_MODULE_AVAILABLE_chowdsp_reflection
