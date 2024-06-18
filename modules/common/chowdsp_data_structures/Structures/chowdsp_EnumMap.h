#pragma once

#if JUCE_MODULE_AVAILABLE_chowdsp_reflection
#include <chowdsp_reflection/chowdsp_reflection.h>

namespace chowdsp
{
/** A map-like container designed to be used with enum keys. */
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

    constexpr EnumMap (const EnumMap&) = delete;
    constexpr EnumMap& operator= (const EnumMap&) = delete;

    constexpr EnumMap (EnumMap&& other) noexcept = default;
    constexpr EnumMap& operator= (EnumMap&& other) noexcept = default;

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

    constexpr void clear()
    {
        for (auto& opt : storage)
            opt.reset();
    }

    constexpr void insert_or_assign (Key key, T value) noexcept
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

    template <typename C = T>
    constexpr std::enable_if_t<std::is_default_constructible_v<C>, T&> emplace (Key key)
    {
        auto& opt = storage[get_index (key)];
        opt.emplace();
        return *opt;
    }

    [[nodiscard]] constexpr std::optional<T>& at (Key key)
    {
        return storage[get_index (key)];
    }

    [[nodiscard]] constexpr const std::optional<T>& at (Key key) const
    {
        return storage[get_index (key)];
    }

    [[nodiscard]] constexpr T& operator[] (Key key) noexcept
    {
        return *at (key);
    }

    [[nodiscard]] constexpr const T& operator[] (Key key) const noexcept
    {
        return *at (key);
    }

    constexpr void erase (Key key)
    {
        at (key).reset();
    }

    [[nodiscard]] constexpr bool contains (Key key) const noexcept
    {
        return at (key).has_value();
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
            do
            {
                ++index;
                ++iter;
            } while (index < std::tuple_size_v<Storage> && ! iter->has_value());

            if (index < std::tuple_size_v<Storage>)
                key = magic_enum::enum_value<Key> (index);
        }

        auto operator*() const
        {
            return std::tie (key, *(*iter));
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
    std::array<std::optional<T>, enum_count> storage;

    constexpr size_t get_index (Key k) const
    {
        return *magic_enum::enum_index (k);
    }
};
} // namespace chowdsp

#endif // JUCE_MODULE_AVAILABLE_chowdsp_reflection
