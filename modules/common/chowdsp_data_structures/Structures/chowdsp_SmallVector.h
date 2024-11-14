#pragma once

#include <variant>

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
 *   - swap
 *   - comparison operators
 *   - overloads for std::erase and std::erase_if
 */
template <typename T, size_t head_size>
class SmallVector
{
public:
    static_assert (std::is_default_constructible_v<T>, "SmallVector currently only supports default-constructible types");

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
            internal_data = ArrayStuff {};
            internal_array().array_size_used = count;
            internal_array().internal_needs_destruction.set();
            std::copy (first, last, internal_array().array.begin());
        }
        else
        {
            internal_data = std::vector<T> { first, last };
        }
    }

    SmallVector (std::initializer_list<T> init) // NOLINT
    {
        if (init.size() <= head_size)
        {
            internal_data = ArrayStuff {};
            internal_array().array_size_used = init.size();
            std::copy (init.begin(), init.end(), internal_array().array.begin());
        }
        else
        {
            internal_data = std::vector<T> { init };
        }
    }

    SmallVector (const SmallVector&) = default;
    SmallVector& operator= (const SmallVector&) = default;
    SmallVector (SmallVector&&) noexcept = default;
    SmallVector& operator= (SmallVector&&) noexcept = default;

    reference at (size_t pos)
    {
        if (usingArray())
            return internal_array().array.at (pos);
        return internal_vector().at (pos);
    }

    const_reference at (size_t pos) const
    {
        if (usingArray())
            return internal_array().array.at (pos);
        return internal_vector().at (pos);
    }

    reference operator[] (size_t pos) noexcept
    {
        if (usingArray())
            return internal_array().array[pos];
        return internal_vector()[pos];
    }

    const_reference operator[] (size_t pos) const noexcept
    {
        if (usingArray())
            return internal_array().array[pos];
        return internal_vector()[pos];
    }

    reference front()
    {
        if (usingArray())
            return internal_array().array.front();
        return internal_vector().front();
    }

    const_reference front() const
    {
        if (usingArray())
            return internal_array().array.front();
        return internal_vector().front();
    }

    reference back()
    {
        if (usingArray())
            return internal_array().array[internal_array().array_size_used - 1];
        return internal_vector().back();
    }

    const_reference back() const
    {
        if (usingArray())
            return internal_array().array[internal_array().array_size_used - 1];
        return internal_vector().back();
    }

    T* data()
    {
        if (usingArray())
            return internal_array().array.data();
        return internal_vector().data();
    }

    const T* data() const
    {
        if (usingArray())
            return internal_array().array.data();
        return internal_vector().data();
    }

    iterator begin() noexcept
    {
        if (usingArray())
            return internal_array().array.data();
        return static_cast<iterator> (&(*internal_vector().begin()));
    }

    const_iterator begin() const noexcept
    {
        if (usingArray())
            return internal_array().array.data();
        return static_cast<const_iterator> (&(*internal_vector().begin()));
    }

    iterator end() noexcept
    {
        if (usingArray())
            return internal_array().array.data() + internal_array().array_size_used;
        return begin() + internal_vector().size();
    }

    const_iterator end() const noexcept
    {
        if (usingArray())
            return internal_array().array.data() + internal_array().array_size_used;
        return begin() + internal_vector().size();
    }

    reverse_iterator rbegin() noexcept
    {
        if (usingArray())
            return static_cast<reverse_iterator> (internal_array().array.data() + internal_array().array_size_used);
        return static_cast<reverse_iterator> (internal_vector().data() + internal_vector().size());
    }

    const_reverse_iterator rbegin() const noexcept
    {
        if (usingArray())
            return static_cast<const_reverse_iterator> (internal_array().array.data() + internal_array().array_size_used);
        return static_cast<const_reverse_iterator> (internal_vector().data() + internal_vector().size());
    }

    reverse_iterator rend() noexcept
    {
        if (usingArray())
            return static_cast<reverse_iterator> (internal_array().array.data());
        return static_cast<reverse_iterator> (internal_vector().data());
    }

    const_reverse_iterator rend() const noexcept
    {
        if (usingArray())
            return static_cast<const_reverse_iterator> (internal_array().array.data());
        return static_cast<const_reverse_iterator> (internal_vector().data());
    }

    [[nodiscard]] bool empty() const noexcept
    {
        if (usingArray())
            return internal_array().array_size_used == 0;
        return internal_vector().empty();
    }

    [[nodiscard]] size_t size() const noexcept
    {
        if (usingArray())
            return internal_array().array_size_used;
        return internal_vector().size();
    }

    [[nodiscard]] size_type max_size() const noexcept
    {
        return internal_vector().max_size();
    }

    void reserve (size_type new_cap)
    {
        if (new_cap <= capacity())
            return;

        if (usingArray())
        {
            move_to_vector();
            internal_vector().reserve (new_cap);
        }
    }

    [[nodiscard]] size_type capacity() const noexcept
    {
        if (usingArray())
            return internal_array().array.size();
        return internal_vector().capacity();
    }

    void shrink_to_fit()
    {
        if (usingArray())
            return;

        if (size() > head_size)
        {
            internal_vector().shrink_to_fit();
            return;
        }

        move_to_array();
    }

    void clear() noexcept
    {
        if (usingArray())
        {
            internal_array().internal_needs_destruction.set();
            for (size_t i = 0; i < internal_array().array_size_used; ++i)
            {
                internal_array().array[i].~T();
                internal_array().internal_needs_destruction[i] = false;
            }
            internal_array().array_size_used = 0;
        }
        else
        {
            internal_vector().clear();
        }
    }

    iterator insert (const_iterator pos, const T& value)
    {
        const auto insert_index = size_t (pos - begin());
        if (usingArray())
        {
            if (internal_array().array_size_used + 1 <= head_size)
            {
                for (size_t idx = internal_array().array_size_used; idx > insert_index; --idx)
                    internal_array().array[idx] = std::move (internal_array().array[idx - 1]);
                internal_array().array[insert_index] = value;
                internal_array().array_size_used++;
                return internal_array().array.data() + insert_index;
            }

            move_to_vector();
        }

        internal_vector().insert (internal_vector().begin() + (int) insert_index, value);
        return internal_vector().data() + insert_index;
    }

    iterator insert (const_iterator pos, T&& value)
    {
        return emplace (pos, std::move (value));
    }

    iterator insert (const_iterator pos, size_t count, const T& value)
    {
        const auto insert_index = size_t (pos - begin());
        if (usingArray())
        {
            if (internal_array().array_size_used + count <= head_size)
            {
                for (size_t idx = internal_array().array_size_used + count - 1; idx > insert_index + count - 1; --idx)
                    internal_array().array[idx] = std::move (internal_array().array[idx - count]);
                std::fill (internal_array().array.begin() + insert_index, internal_array().array.begin() + insert_index + count, value);
                internal_array().array_size_used += count;
                return internal_array().array.data() + insert_index;
            }

            move_to_vector();
        }

        internal_vector().insert (internal_vector().begin() + (int) insert_index, count, value);
        return internal_vector().data() + insert_index;
    }

    template <class InputIt>
    iterator insert (const_iterator pos, InputIt first, InputIt last)
    {
        const auto num_to_insert = size_t (last - first);
        const auto insert_index = size_t (pos - begin());
        if (usingArray())
        {
            if (internal_array().array_size_used + num_to_insert <= head_size)
            {
                for (size_t idx = internal_array().array_size_used + num_to_insert - 1; idx > insert_index + num_to_insert - 1; --idx)
                    internal_array().array[idx] = std::move (internal_array().array[idx - num_to_insert]);
                std::copy (first, last, internal_array().array.begin() + insert_index);
                internal_array().array_size_used += num_to_insert;
                return internal_array().array.data() + insert_index;
            }

            move_to_vector();
        }

        internal_vector().insert (internal_vector().begin() + (int) insert_index, first, last);
        return internal_vector().data() + insert_index;
    }

    iterator insert (const_iterator pos, std::initializer_list<T> ilist)
    {
        return insert (pos, ilist.begin(), ilist.end());
    }

    iterator emplace (const_iterator pos, T&& value)
    {
        const auto insert_index = size_t (pos - begin());
        if (usingArray())
        {
            if (internal_array().array_size_used + 1 <= head_size)
            {
                for (size_t idx = internal_array().array_size_used; idx > insert_index; --idx)
                    internal_array().array[idx] = std::move (internal_array().array[idx - 1]);
                internal_array().array[insert_index] = std::move (value);
                internal_array().array_size_used++;
                return internal_array().array.data() + insert_index;
            }

            move_to_vector();
        }

        internal_vector().emplace (internal_vector().begin() + (int) insert_index, std::move (value));
        return internal_vector().data() + insert_index;
    }

    iterator erase (iterator pos)
    {
        return erase (static_cast<const_iterator> (pos));
    }

    iterator erase (const_iterator pos)
    {
        const auto erase_index = size_t (pos - begin());
        if (usingArray())
        {
            for (size_t idx = erase_index; idx < internal_array().array_size_used - 1; ++idx)
                internal_array().array[idx] = std::move (internal_array().array[idx + 1]);
            internal_array().array_size_used--;
            return internal_array().array.data() + erase_index;
        }

        internal_vector().erase (internal_vector().begin() + (int) erase_index);
        return internal_vector().data() + erase_index;
    }

    iterator erase (iterator first, iterator last)
    {
        return erase (static_cast<const_iterator> (first), static_cast<const_iterator> (last));
    }

    iterator erase (const_iterator first, const_iterator last)
    {
        const auto num_to_erase = size_t (last - first);
        const auto first_erase_index = size_t (first - begin());
        if (usingArray())
        {
            for (size_t idx = first_erase_index; idx < internal_array().array_size_used - num_to_erase; ++idx)
                internal_array().array[idx] = std::move (internal_array().array[idx + num_to_erase]);
            internal_array().array_size_used -= num_to_erase;
            return internal_array().array.data() + first_erase_index;
        }

        internal_vector().erase (internal_vector().begin() + (int) first_erase_index,
                                 internal_vector().begin() + (int) first_erase_index + (int) num_to_erase);
        return internal_vector().data() + first_erase_index;
    }

    void push_back (const T& value)
    {
        if (usingArray())
        {
            if (internal_array().array_size_used + 1 <= head_size)
            {
                internal_array().array[internal_array().array_size_used] = value;
                internal_array().array_size_used++;
                return;
            }

            move_to_vector();
        }

        internal_vector().push_back (value);
    }

    void push_back (T&& value)
    {
        if (usingArray())
        {
            if (internal_array().array_size_used + 1 <= head_size)
            {
                internal_array().array[internal_array().array_size_used] = std::move (value);
                internal_array().array_size_used++;
                return;
            }

            move_to_vector();
        }

        internal_vector().push_back (std::move (value));
    }

    template <class... Args>
    reference emplace_back (Args&&... args)
    {
        if (usingArray())
        {
            if (internal_array().array_size_used + 1 <= head_size)
            {
                if (internal_array().internal_needs_destruction[internal_array().array_size_used])
                    internal_array().array[internal_array().array_size_used].~T();
                new (&internal_array().array[internal_array().array_size_used]) T (args...);
                internal_array().array_size_used++;
                return internal_array().array[internal_array().array_size_used - 1];
            }

            move_to_vector();
        }

        return internal_vector().emplace_back (std::forward<Args> (args)...);
    }

    void pop_back()
    {
        if (usingArray())
            internal_array().array_size_used--;
        else
            internal_vector().pop_back();
    }

    void resize (size_t count, const T& value = {})
    {
        if (count == size())
            return;

        if (count <= head_size)
        {
            internal_data = ArrayStuff {};
            internal_array().array_size_used = count;
            std::fill (internal_array().array.begin(), internal_array().array.begin() + count, value);
        }
        else
        {
            internal_data = std::vector<T> {};
            internal_vector().resize (count, value);
        }
    }

private:
    void move_to_vector()
    {
        std::vector<T> moving {};
        if constexpr (std::is_trivially_copyable_v<T>)
        {
            moving.resize (internal_array().array_size_used);
            std::copy (internal_array().array.begin(), internal_array().array.begin() + internal_array().array_size_used, moving.begin());
        }
        else
        {
            for (size_t count = 0; count < internal_array().array_size_used; ++count)
                moving.push_back (std::move (internal_array().array[count]));
        }
        internal_data = std::move (moving);
    }

    void move_to_array()
    {
        ArrayStuff moving {};
        moving.array_size_used = internal_vector().size();
        if constexpr (std::is_trivially_copyable_v<T>)
        {
            std::copy (internal_vector().begin(), internal_vector().end(), moving.array.begin());
        }
        else
        {
            for (size_t count = 0; count < moving.array_size_used; ++count)
                moving.array[count] = std::move (internal_vector()[count]);
        }
        internal_data = std::move (moving);
    }

    [[nodiscard]] bool usingArray() const noexcept
    {
        return internal_data.index() == 0;
    }

    auto& internal_array()
    {
        return std::get<0> (internal_data);
    }

    const auto& internal_array() const
    {
        return std::get<0> (internal_data);
    }

    auto& internal_vector()
    {
        return std::get<1> (internal_data);
    }

    const auto& internal_vector() const
    {
        return std::get<1> (internal_data);
    }

    struct ArrayStuff
    {
        ArrayStuff() = default;
        std::array<T, head_size> array {};
        std::bitset<head_size> internal_needs_destruction {};
        size_t array_size_used = 0;
    };
    std::variant<ArrayStuff, std::vector<T>> internal_data;
};
} // namespace chowdsp
