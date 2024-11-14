#pragma once

namespace chowdsp
{
/**
 * A data structure similar to std::array<std::optional<T>, N>, but with less memory overhead.
 *
 * This implementation is still a work-in-progress.
 */
template <typename T, size_t N>
class OptionalArray
{
    std::array<RawObject<T>, N> objects {};
    std::bitset<N> optional_flags {};

public:
    using value_type = T;
    static constexpr auto max_size = N;

    OptionalArray() = default;
    OptionalArray (const OptionalArray&) = default;
    OptionalArray& operator= (const OptionalArray&) = default;
    OptionalArray (OptionalArray&&) noexcept = default;
    OptionalArray& operator= (OptionalArray&&) noexcept = default;

    ~OptionalArray()
    {
        for (auto [idx, object] : enumerate (objects))
        {
            if (optional_flags[idx])
                object.destruct();
        }
    }

    [[nodiscard]] bool has_value (size_t idx) const noexcept
    {
        return optional_flags[idx];
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return optional_flags.none();
    }

    [[nodiscard]] size_t count_values() const noexcept
    {
        return optional_flags.count();
    }

    OptionalRef<T> operator[] (size_t idx)
    {
        if (! optional_flags[idx])
            return {};
        return objects[idx].item();
    }

    OptionalRef<const T> operator[] (size_t idx) const
    {
        if (! optional_flags[idx])
            return {};
        return objects[idx].item();
    }

    template <typename... Args>
    T& emplace (size_t idx, Args&&... args)
    {
        if (optional_flags[idx])
            objects[idx].destruct();
        optional_flags[idx] = true;
        return *objects[idx].construct (std::forward<Args> (args)...);
    }

    void erase (size_t idx)
    {
        if (optional_flags[idx])
        {
            objects[idx].destruct();
            optional_flags[idx] = false;
        }
    }

    template <bool is_const = false>
    struct iterator
    {
        size_t index {};
        std::conditional_t<is_const, const OptionalArray&, OptionalArray&> array {};

        bool operator!= (const iterator& other) const noexcept
        {
            return &array != &other.array || index != other.index;
        }
        void operator++() noexcept
        {
            do
            {
                ++index;
            } while (index < N && ! array.optional_flags[index]);
        }
        auto& operator*() const noexcept
        {
            return array.objects[index].item();
        }
    };

    auto begin()
    {
        return iterator<> { 0, *this };
    }

    auto end()
    {
        return iterator<> { N, *this };
    }

    auto begin() const
    {
        return cbegin();
    }

    auto end() const
    {
        return cend();
    }

    auto cbegin() const
    {
        return iterator<true> { 0, *this };
    }

    auto cend() const
    {
        return iterator<true> { N, *this };
    }
};
} // namespace chowdsp
