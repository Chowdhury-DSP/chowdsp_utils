#pragma once

namespace chowdsp
{
template <typename KeyType, typename ValueType>
class SmallMap
{
public:
    using key_type = KeyType;
    using mapped_type = ValueType;
    using value_type = std::pair<const KeyType, ValueType>;

    SmallMap() = default;
    SmallMap(const SmallMap&) = default;
    SmallMap& operator= (const SmallMap&) = default;
    SmallMap(SmallMap&&) noexcept = default;
    SmallMap& operator= (SmallMap&&) noexcept = default;

    explicit SmallMap (std::initializer_list<value_type> init)
    {
        reserve (init.size());
        for (auto& [key, value] : init)
            insert_or_assign (key, value);
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return keys.empty();
    }

    [[nodiscard]] size_t size() const noexcept
    {
        return keys.size();
    }

    void reserve (size_t n)
    {
        keys.reserve (n);
        values.reserve (n);
    }

    void clear()
    {
        keys.clear();
        values.clear();
    }

    void shrink_to_fit()
    {
        keys.shrink_to_fit();
        values.shrink_to_fit();
    }

    void insert_or_assign (const KeyType& key, const ValueType& value)
    {
        if (auto index = find (key))
        {
            values[*index] = value;
            return;
        }

        keys.push_back (key);
        values.push_back (value);
    }

    void insert_or_assign (const KeyType& key, ValueType&& value)
    {
        if (auto index = find (key))
        {
            values[*index] = std::move (value);
            return;
        }

        keys.push_back (key);
        values.push_back (std::move (value));
    }

    ValueType* get (size_t index)
    {
        jassert (index < size());
        return &values[index];
    }

    ValueType* get (const KeyType& key)
    {
        if (auto index = find (key))
            return get (*index);
        return nullptr;
    }

    ValueType* operator[] (const KeyType& key)
    {
        return get (key);
    }

    std::optional<size_t> find (const KeyType& key) const noexcept
    {
        const auto key_iter = std::find (keys.begin(), keys.end(), key);
        if (key_iter == keys.end())
            return std::nullopt;
        return std::distance (keys.begin(), key_iter);
    }

    bool contains (const KeyType& key) const noexcept
    {
        return find (key).has_value();
    }

    auto begin()
    {
        return zip (keys, values).begin();
    }

    auto end()
    {
        return zip (keys, values).end();
    }

    void erase (const KeyType& key)
    {
        if (auto index = find (key))
        {
            keys.erase (keys.begin() + *index);
            values.erase (values.begin() + *index);
        }
    }

private:
    std::vector<KeyType> keys {};
    std::vector<ValueType> values {};
};
}
