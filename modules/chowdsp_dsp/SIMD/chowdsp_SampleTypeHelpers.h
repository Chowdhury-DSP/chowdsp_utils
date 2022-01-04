#pragma once

#include <type_traits>

/** Useful structs for determining the internal data type of SIMD types */
namespace chowdsp::SampleTypeHelpers
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

/** Useful struct for determining if a type is a SIMDRegister */
template <typename T, typename NumericType = typename ElementType<T>::Type, typename SIMDType = juce::dsp::SIMDRegister<NumericType>>
inline constexpr bool IsSIMDRegister = std::is_same<T, SIMDType>::value;
} // namespace chowdsp::SampleTypeHelpers
