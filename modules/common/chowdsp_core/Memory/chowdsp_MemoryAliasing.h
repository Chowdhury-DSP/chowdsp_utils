#pragma once

namespace chowdsp
{
/** Utility functions for working with raw memory */
namespace MemoryUtils
{
    /**
     * Returns true if the memory in the range [x1Start, x1End) aliases
     * with the memory in the range [x2Start, x2End).
     */
    inline bool doesMemoryAlias (std::uintptr_t x1Start, std::uintptr_t x1End, std::uintptr_t x2Start, std::uintptr_t x2End)
    {
        return x1Start < x2End && x2Start < x1End;
    }

    /**
     * Returns true if the memory in the range [x1, x1 + x1Num) aliases
     * with the memory in the range [x2, x2 + x2Num).
     */
    template <typename T, typename IntType>
    bool doesMemoryAlias (const T* const x1, IntType x1Num, const T* const x2, IntType x2Num)
    {
        return doesMemoryAlias (reinterpret_cast<std::uintptr_t> (x1), // NOSONAR (casting to uintptr_t should be safe here)
                                reinterpret_cast<std::uintptr_t> (x1 + x1Num), // NOSONAR
                                reinterpret_cast<std::uintptr_t> (x2), // NOSONAR
                                reinterpret_cast<std::uintptr_t> (x2 + x2Num)); // NOSONAR
    }

    /** Returns true if the memory stored in the two containers aliases. */
    template <typename ContainerType1, typename ContainerType2>
    bool doesMemoryAlias (const ContainerType1& x1, const ContainerType2& x2)
    {
        return doesMemoryAlias (std::data (x1), std::size (x1), std::data (x2), std::size (x2));
    }

    /**
     * Returns true if the memory stored between begin1 and end1 aliases with
     * the memory stored between begin2 and end2.
     */
    template <typename IterType>
    bool doesMemoryAlias (const IterType& begin1, const IterType& end1, const IterType& begin2, const IterType& end2)
    {
        return doesMemoryAlias (&(*begin1), std::distance (begin1, end1), &(*begin2), std::distance (begin2, end2));
    }
} // namespace MemoryUtils
} // namespace chowdsp
