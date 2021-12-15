#pragma once

#include <type_traits>

namespace chowdsp
{
/** Useful structs for determining the internal data type of SIMD types */
namespace SampleTypeHelpers
{
    template <typename T, bool = std::is_floating_point<T>::value>
    struct ElementType
    {
        using Type = T;
    };

    template <typename T>
    struct ElementType<T, false>
    {
        using Type = typename T::value_type;
    };
} // namespace SampleTypeHelpers
} // namespace chowdsp
