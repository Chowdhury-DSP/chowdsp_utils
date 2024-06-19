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

    EnumMap (std::initializer_list<std::pair<Key, T>> init)
    {
        for (auto& [key, value] : init)
            insert_or_assign (key, value);
    }

    EnumMap (const EnumMap&) = delete;
    EnumMap& operator= (const EnumMap&) = delete;

    EnumMap (EnumMap&& other) noexcept = default;
    EnumMap& operator= (EnumMap&& other) noexcept = default;

    [[nodiscard]] bool empty() const noexcept
    {
        return init_flags.none();
    }

    [[nodiscard]] size_t size() const noexcept
    {
        return init_flags.count();
    }

    [[nodiscard]] static constexpr size_t max_size() noexcept
    {
        return enum_count;
    }

    void clear()
    {
        for (auto [idx, object] : enumerate (storage))
        {
            if (init_flags[idx])
                object.destruct();
        }
        init_flags.reset();
    }

    void insert_or_assign (Key key, T value) noexcept
    {
        auto idx = get_index (key);
        if (init_flags[idx])
            storage[idx].destruct();

        storage[idx].construct (value);
        init_flags[idx] = true;
    }

    template <typename... Args>
    T& emplace (Key key, Args&&... args)
    {
        auto idx = get_index (key);
        auto& object = storage[idx];
        if (init_flags[idx])
            object.destruct();

        object.construct (std::forward<Args> (args)...);
        init_flags[idx] = true;
        return object.item();
    }

    template <typename C = T>
    std::enable_if_t<std::is_default_constructible_v<C>, T&> emplace (Key key)
    {
        auto idx = get_index (key);
        auto& object = storage[idx];
        if (init_flags[idx])
            object.destruct();

        object.construct();
        init_flags[idx] = true;
        return object.item();
    }

    [[nodiscard]] OptionalRef<T> at (Key key)
    {
        auto idx = get_index (key);
        if (! init_flags[idx])
            return std::nullopt;
        return storage[idx].item();
    }

    [[nodiscard]] OptionalRef<const T> at (Key key) const
    {
        auto idx = get_index (key);
        if (! init_flags[idx])
            return std::nullopt;
        return storage[idx].item();
    }

    [[nodiscard]] T& operator[] (Key key) noexcept
    {
        return storage[get_index (key)].item();
    }

    [[nodiscard]] const T& operator[] (Key key) const noexcept
    {
        return storage[get_index (key)].item();
    }

    void erase (Key key)
    {
        auto idx = get_index (key);
        if (init_flags[idx])
        {
            storage[idx].destruct();
            init_flags[idx] = false;
        }
    }

    [[nodiscard]] bool contains (Key key) const noexcept
    {
        return init_flags[get_index (key)];
    }

    template <bool is_const = false>
    struct iterator
    {
        size_t index;
        std::conditional_t<is_const, const EnumMap&, EnumMap&> map;
        Key key;

        bool operator!= (const iterator& other) const
        {
            return &map == &other.map && index != other.index;
        }

        void operator++()
        {
            do
            {
                ++index;
            } while (index < enum_count && ! map.init_flags[index]);

            if (index < enum_count)
                key = magic_enum::enum_value<Key> (index);
        }

        auto operator*() const
        {
            return std::tie (key, map.storage[index].item());
        }
    };

    auto begin() noexcept
    {
        return iterator<> {
            0,
            *this,
            magic_enum::enum_value<Key> (0),
        };
    }

    auto begin() const noexcept
    {
        return iterator<true> {
            0,
            *this,
            magic_enum::enum_value<Key> (0),
        };
    }

    auto end() noexcept
    {
        return iterator<> {
            enum_count,
            *this,
            {},
        };
    }

    auto end() const noexcept
    {
        return iterator<true> {
            enum_count,
            *this,
            {},
        };
    }

private:
    static constexpr auto enum_count = magic_enum::enum_count<Key>();
    std::array<RawObject<T>, enum_count> storage {};
    std::bitset<enum_count> init_flags {};

    size_t get_index (Key k) const
    {
        return *magic_enum::enum_index (k);
    }
};
} // namespace chowdsp

#endif // JUCE_MODULE_AVAILABLE_chowdsp_reflection
