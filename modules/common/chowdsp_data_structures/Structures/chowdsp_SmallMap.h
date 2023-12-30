#pragma once

namespace chowdsp
{
/**
 * A vector-backed map.
 *
 * At the moment, this implementation behaves
 * somewhat similar to std::unordered_map, but
 * _does not_ maintain reference stability.
 */
template <typename KeyType, typename ValueType>
class SmallMap
{
public:
    using key_type = KeyType;
    using mapped_type = ValueType;
    using value_type = std::pair<const KeyType, ValueType>;

    SmallMap() = default;
    SmallMap (const SmallMap&) = default;
    SmallMap& operator= (const SmallMap&) = default;
    SmallMap (SmallMap&&) noexcept = default;
    SmallMap& operator= (SmallMap&&) noexcept = default;

    /** Initializes a map with an initial state. */
    SmallMap (std::initializer_list<value_type> init) // NOLINT
    {
        reserve (init.size());
        for (auto& [key, value] : init)
            insert_or_assign (key, value);
    }

    /** Returns true if the map is empty. */
    [[nodiscard]] bool empty() const noexcept
    {
        return keys.empty();
    }

    /** Returns the size of the map. */
    [[nodiscard]] size_t size() const noexcept
    {
        return keys.size();
    }

    /** Reserves space in the vectors that support the map. This may invalidate references. */
    void reserve (size_t n)
    {
        keys.reserve (n);
        values.reserve (n);
    }

    /** Clears the map. */
    void clear()
    {
        keys.clear();
        values.clear();
    }

    /** Calls shrink_to_fit on the underlying vectors. */
    void shrink_to_fit()
    {
        keys.shrink_to_fit();
        values.shrink_to_fit();
    }

    /**
     * Inserts or assigns a value to a key in the map.
     * This may invalidate references.
     */
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

    /**
     * Inserts or assigns a value to a key in the map.
     * This may invalidate references.
     */
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

    /**
     * Returns the value at a given index in the underlying vector.
     * This method is not bounds-checked, so be only use this if you're sure
     * you know what you're doing.
     */
    ValueType& get (size_t index)
    {
        jassert (index < size());
        return values[index];
    }

    /**
     * Returns a pointer to the value for a given key,
     * or nullptr, if the key is not found in the map.
     */
    ValueType* get (const KeyType& key)
    {
        if (auto index = find (key))
            return &get (*index);
        return nullptr;
    }

    /**
     * Returns a pointer to the value for a given key,
     * or nullptr, if the key is not found in the map.
     */
    ValueType* operator[] (const KeyType& key)
    {
        return get (key);
    }

    /**
     * Returns an index to a given key in the map,
     * or std::nullopt if the key is not present in the map.
     */
    std::optional<size_t> find (const KeyType& key) const noexcept
    {
        const auto key_iter = std::find (keys.begin(), keys.end(), key);
        if (key_iter == keys.end())
            return std::nullopt;
        return std::distance (keys.begin(), key_iter);
    }

    /** Returns true if the key is present in the map. */
    bool contains (const KeyType& key) const noexcept
    {
        return find (key).has_value();
    }

    /**
     * Erases an element from the map.
     * This may invalidate references.
     */
    void erase (const KeyType& key)
    {
        if (auto index = find (key))
        {
            keys.erase (keys.begin() + static_cast<int> (*index));
            values.erase (values.begin() + static_cast<int> (*index));
        }
    }

    /** Begin iterator. */
    auto begin()
    {
        return zip (keys, values).begin();
    }

    /** End iterator. */
    auto end()
    {
        return zip (keys, values).end();
    }

private:
    std::vector<KeyType> keys {};
    std::vector<ValueType> values {};
};
} // namespace chowdsp
