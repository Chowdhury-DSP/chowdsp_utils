#pragma once

namespace chowdsp
{
/** Helpful methods for working with atomic data types */
namespace AtomicHelpers
{
    /**
     * Compares an atomic variable to some compareVal, and sets the atomic
     * variable to a new value if the comparison returns true.
     */
    template <typename T, typename atomic_type = std::atomic<T>>
    bool compareExchange (atomic_type& atomic_val, T compareVal, T valToSetIfTrue) noexcept
    {
        return atomic_val.compare_exchange_strong (compareVal, valToSetIfTrue);
    }

    /** Simplified version of compareExchange for boolean types */
    template <typename T = bool, typename atomic_type = std::atomic<T>>
    bool compareNegate (atomic_type& atomic_val, T compareVal = (T) true) noexcept
    {
        return compareExchange (atomic_val, compareVal, ! compareVal);
    }

    /**
     * Implementation of std::fetch_or implemented for std::atomic_bool
     *
     * Reference: https://stackoverflow.com/questions/29390247/stdatomicbool-fetch-and-and-fetch-or-realization
     */
    inline bool fetch_or (std::atomic_bool& atom, bool val)
    {
        bool bRes = ! val;
        atom.compare_exchange_strong (bRes, true);
        return bRes;
    }

    /**
     * Implementation of std::fetch_and implemented for std::atomic_bool
     *
     * Reference: https://stackoverflow.com/questions/29390247/stdatomicbool-fetch-and-and-fetch-or-realization
     */
    inline bool fetch_and (std::atomic_bool& atom, bool val)
    {
        bool bRes = true;
        atom.compare_exchange_strong (bRes, val);
        return bRes;
    }
} // namespace AtomicHelpers

/**
 * std::atomic_ref was introduced in C++20, but for simple atomic types
 * this implementation should work okay.
 */
template <typename T>
struct AtomicRef
{
    static constexpr auto is_non_const = ! std::is_const_v<T>;
    using atomic_type = std::conditional_t<is_non_const, std::atomic<T>, const std::atomic<std::remove_const_t<T>>>;

    static_assert (atomic_type::is_always_lock_free, "Base atomic type must be lock free.");
    static_assert (sizeof (T) == sizeof (atomic_type), "Incompatible layout.");
    static_assert (alignof (T) == alignof (atomic_type), "Incompatible layout.");

    explicit AtomicRef (T& val) : atomic_val (reinterpret_cast<atomic_type&> (val)) {} // NOSONAR (if the above static_assert's pass, then reinterpret_cast should be safe)
    AtomicRef (const AtomicRef&) = delete;
    AtomicRef& operator= (const AtomicRef&) = delete;

    /** Performs std::atomic<T>::store */
    template <bool non_const = is_non_const>
    inline std::enable_if_t<non_const, void>
        store (T val)
    {
        atomic_val.store (val);
    }

    /** Performs std::atomic<T>::load */
    inline T load() const
    {
        return atomic_val.load();
    }

    /** Performs std::atomic<T>::compare_exchange_strong */
    template <bool non_const = is_non_const>
    inline std::enable_if_t<non_const, bool>
        compare_exchange_strong (T& expected, T desired)
    {
        return atomic_val.compare_exchange_strong (expected, desired);
    }

private:
    atomic_type& atomic_val;
};
} // namespace chowdsp
