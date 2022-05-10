#pragma once

namespace chowdsp
{
namespace AtomicHelpers
{
    /** Simplified wrapped of std::atomic<T>::compare_exchange_strong */
    template <typename T>
    bool compareExchange (std::atomic<T>& atomic_val, T compareVal, T valToSetIfTrue) noexcept
    {
        return atomic_val.compare_exchange_strong (compareVal, valToSetIfTrue);
    }
} // namespace AtomicHelpers

/**
 * std::atomic_ref was introduced in C++20, but for simple atomic types
 * this implementation should work okay.
 */
template <typename T>
struct AtomicRef
{
    using atomic_type = std::conditional_t<std::is_const_v<T>, const std::atomic<std::remove_const_t<T>>, std::atomic<T>>;

    static_assert (atomic_type::is_always_lock_free, "Base atomic type must be lock free.");
    static_assert (sizeof (T) == sizeof (atomic_type), "Incompatible layout.");
    static_assert (alignof (T) == alignof (atomic_type), "Incompatible layout.");

    explicit AtomicRef (T& val) : atomic_val (reinterpret_cast<atomic_type&> (val))
    {
    }

    /** Performs std::atomic<T>::store */
    inline void store (T val)
    {
        atomic_val.store (val);
    }

    /** Performs std::atomic<T>::load */
    inline T load() const
    {
        return atomic_val.load();
    }

    /** Performs std::atomic<T>::compare_exchange_strong */
    inline bool compare_exchange_strong (T& expected, T desired)
    {
        return atomic_val.compare_exchange_strong (expected, desired);
    }

private:
    atomic_type& atomic_val;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AtomicRef)
};
} // namespace chowdsp
