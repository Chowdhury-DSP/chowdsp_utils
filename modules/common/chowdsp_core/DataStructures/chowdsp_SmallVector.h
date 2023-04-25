#pragma once

namespace chowdsp
{
/**
 * A (mostly) standards-compliant replacement for std::vector,
 * but with a small-string-style optimization.
 *
 * If a vector grows large enough to need dynamic storage, and
 * then later becomes small enough to once again fit into static
 * storage, you can do this explicitly with shrink_to_fit(). Otherwise,
 * the data will remain in dynamic storage.
 *
 * The following methods from std::vector are not implemented:
 *   - insert (single value copy)
 *   - insert (single value move)
 *   - insert (range)
 *   - insert (initializer list)
 *   - emplace
 *   - erase (single value)
 *   - erase (range)
 *   - swap (not sure I'm going to do this one)
 *   - comparison operators
 *   - overloads for std::erase and std::erase_if
 */
template <typename T, size_t head_size>
class SmallVector
{
public:
    using value_type = T;
    using allocator_type = typename std::vector<T>::allocator_type;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = typename std::allocator_traits<allocator_type>::pointer;
    using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    SmallVector() = default;
    ~SmallVector() = default;

    explicit SmallVector (size_t count, const T& value = T())
    {
        resize (count, value);
    }

    template <class InputIt>
    SmallVector (InputIt first, InputIt last)
    {
        const auto count = size_t (last - first);

        if (count <= head_size)
        {
            usingArray = true;
            internal_array_size_used = count;
            std::copy (first, last, internal_array.begin());
        }
        else
        {
            usingArray = false;
            internal_vector = std::vector<T> { first, last };
        }
    }

    SmallVector (std::initializer_list<T> init)
    {
        if (init.size() <= head_size)
        {
            usingArray = true;
            internal_array_size_used = init.size();
            std::copy (init.begin(), init.end(), internal_array.begin());
        }
        else
        {
            usingArray = false;
            internal_vector = std::vector<T> { init };
        }
    }

    SmallVector (const SmallVector&) = default;
    SmallVector& operator= (const SmallVector&) = default;
    SmallVector (SmallVector&&) noexcept = default;
    SmallVector& operator= (SmallVector&&) noexcept = default;

    reference at (size_t pos)
    {
        if (usingArray)
            return internal_array.at (pos);
        return internal_vector.at (pos);
    }

    const_reference at (size_t pos) const
    {
        if (usingArray)
            return internal_array.at (pos);
        return internal_vector.at (pos);
    }

    reference operator[] (size_t pos) noexcept
    {
        if (usingArray)
            return internal_array[pos];
        return internal_vector[pos];
    }

    const_reference operator[] (size_t pos) const noexcept
    {
        if (usingArray)
            return internal_array[pos];
        return internal_vector[pos];
    }

    reference front()
    {
        if (usingArray)
            return internal_array.front();
        return internal_vector.front();
    }

    const_reference front() const
    {
        if (usingArray)
            return internal_array.front();
        return internal_vector.front();
    }

    reference back()
    {
        if (usingArray)
            return internal_array[internal_array_size_used - 1];
        return internal_vector.back();
    }

    const_reference back() const
    {
        if (usingArray)
            return internal_array[internal_array_size_used - 1];
        return internal_vector.back();
    }

    T* data()
    {
        if (usingArray)
            return internal_array.data();
        return internal_vector.data();
    }

    const T* data() const
    {
        if (usingArray)
            return internal_array.data();
        return internal_vector.data();
    }

    iterator begin() noexcept
    {
        if (usingArray)
            return internal_array.begin();
        return static_cast<iterator> (&(*internal_vector.begin()));
    }

    const_iterator begin() const noexcept
    {
        if (usingArray)
            return internal_array.begin();
        return static_cast<const_iterator> (&(*internal_vector.begin()));
    }

    iterator end() noexcept
    {
        if (usingArray)
            return internal_array.begin() + internal_array_size_used;
        return static_cast<iterator> (&(*internal_vector.end()));
    }

    const_iterator end() const noexcept
    {
        if (usingArray)
            return internal_array.begin() + internal_array_size_used;
        return static_cast<const_iterator> (&(*internal_vector.end()));
    }

    reverse_iterator rbegin() noexcept
    {
        if (usingArray)
            return internal_array.rend() - (typename reverse_iterator::difference_type) internal_array_size_used;
        return static_cast<reverse_iterator> (&(*internal_vector.rbegin()));
    }

    const_reverse_iterator rbegin() const noexcept
    {
        if (usingArray)
            return internal_array.rend() - (typename reverse_iterator::difference_type) internal_array_size_used;
        return static_cast<const_reverse_iterator> (&(*internal_vector.rbegin()));
    }

    reverse_iterator rend() noexcept
    {
        if (usingArray)
            return internal_array.rend();
        return static_cast<reverse_iterator> (&(*internal_vector.rend()));
    }

    const_reverse_iterator rend() const noexcept
    {
        if (usingArray)
            return internal_array.rend();
        return static_cast<const_reverse_iterator> (&(*internal_vector.rend()));
    }

    [[nodiscard]] bool empty() const noexcept
    {
        if (usingArray)
            return internal_array_size_used == 0;
        return internal_vector.empty();
    }

    [[nodiscard]] size_t size() const noexcept
    {
        if (usingArray)
            return internal_array_size_used;
        return internal_vector.size();
    }

    [[nodiscard]] size_type max_size() const noexcept
    {
        return internal_vector.max_size();
    }

    void reserve (size_type new_cap)
    {
        if (new_cap <= capacity())
            return;

        internal_vector.reserve (new_cap);
        if (usingArray)
            move_to_vector();
    }

    [[nodiscard]] size_type capacity() const noexcept
    {
        if (usingArray)
            return internal_array.size();
        return internal_vector.capacity();
    }

    void shrink_to_fit()
    {
        if (usingArray)
            return;

        if (size() > head_size)
        {
            internal_vector.shrink_to_fit();
            return;
        }

        move_to_array();
    }

    void clear() noexcept
    {
        if (usingArray)
            internal_array_size_used = 0;
        internal_vector.clear();
    }

    //    iterator insert (const_iterator pos, const T& value)
    //    {
    //        if (usingArray)
    //        {
    //            const auto insert_index = pos - internal_array.begin();
    //            if (internal_array_size_used + 1 <= head_size)
    //            {
    //                for (size_t idx = internal_array_size_used; idx > insert_index; --idx)
    //                    internal_array[idx] = std::move (internal_array[idx-1]);
    //                internal_array[insert_index] = value;
    //                internal_array_size_used++;
    //                return internal_array.begin() + insert_index;
    //            }
    //
    //            move_to_vector();
    //            pos = internal_vector.begin() + insert_index;
    //        }
    //
    //        return internal_vector.insert (pos, value);
    //    }

    void push_back (const T& value)
    {
        if (usingArray)
        {
            if (internal_array_size_used + 1 <= head_size)
            {
                internal_array[internal_array_size_used] = value;
                internal_array_size_used++;
                return;
            }

            move_to_vector();
        }

        internal_vector.push_back (value);
    }

    void push_back (T&& value)
    {
        if (usingArray)
        {
            if (internal_array_size_used + 1 <= head_size)
            {
                internal_array[internal_array_size_used] = std::move (value);
                internal_array_size_used++;
                return;
            }

            move_to_vector();
        }

        internal_vector.push_back (std::move (value));
    }

    template <class... Args>
    reference emplace_back (Args&&... args)
    {
        if (usingArray)
        {
            if (internal_array_size_used + 1 <= head_size)
            {
                new (&internal_array[internal_array_size_used]) T (args...);
                internal_array_size_used++;
                return internal_array[internal_array_size_used - 1];
            }

            move_to_vector();
        }

        return internal_vector.emplace_back (std::forward<Args> (args)...);
    }

    void pop_back()
    {
        if (usingArray)
            internal_array_size_used--;
        else
            internal_vector.pop_back();
    }

    void resize (size_t count, const T& value = {})
    {
        if (count == size())
            return;

        if (count <= head_size)
        {
            usingArray = true;
            internal_array_size_used = count;
            std::fill (internal_array.begin(), internal_array.begin() + count, value);
            internal_vector.clear();
        }
        else
        {
            usingArray = false;
            internal_vector.resize (count, value);
        }
    }

private:
    void move_to_vector()
    {
        usingArray = false;
        if constexpr (std::is_trivially_copyable_v<T>)
        {
            internal_vector.resize (internal_array_size_used);
            std::copy (internal_array.begin(), internal_array.begin() + internal_array_size_used, internal_vector.begin());
        }
        else
        {
            internal_vector.clear();
            for (size_t count = 0; count < internal_array_size_used; ++count)
                internal_vector.push_back (std::move (internal_array[count]));
        }
    }

    void move_to_array()
    {
        usingArray = true;
        internal_array_size_used = internal_vector.size();
        if constexpr (std::is_trivially_copyable_v<T>)
        {
            std::copy (internal_vector.begin(), internal_vector.end(), internal_array.begin());
        }
        else
        {
            for (size_t count = 0; count < internal_array_size_used; ++count)
                internal_array[count] = std::move (internal_vector[count]);
        }
        internal_vector.clear();
    }

    std::array<T, head_size> internal_array {};
    size_t internal_array_size_used = 0;

    std::vector<T> internal_vector {};

    bool usingArray = true;

    JUCE_LEAK_DETECTOR (SmallVector)
};
} // namespace chowdsp
