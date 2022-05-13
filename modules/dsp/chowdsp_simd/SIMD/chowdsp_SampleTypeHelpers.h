#pragma once

#include <type_traits>

/** Useful structs for determining the internal data type of SIMD types */
namespace chowdsp::SampleTypeHelpers
{
template <typename T, bool = std::is_floating_point_v<T> || std::is_same_v<T, std::complex<float>> || std::is_same_v<T, std::complex<double>>>
struct ElementType
{
    using Type = T;
    static constexpr int Size = 1;
};

template <typename T>
struct ElementType<xsimd::batch<T>, false>
{
    using batch_type = xsimd::batch<T>;
    using Type = typename batch_type::value_type;
    static constexpr int Size = (int) batch_type::size;
};

template <typename T>
struct ElementType<const xsimd::batch<T>, false>
{
    using batch_type = xsimd::batch<T>;
    using Type = const typename batch_type::value_type;
    static constexpr int Size = (int) batch_type::size;
};

/** Type alias for a SIMD numeric type */
template <typename SampleType>
using NumericType = typename ElementType<SampleType>::Type;

/** Type alias for retrieving a SIMD numeric type for a processor that has one defined */
template <typename ProcessorType>
using ProcessorNumericType = typename ProcessorType::NumericType;

/** Useful template expression for determining if a type is a SIMDRegister */
template <typename T, typename NumericType = NumericType<T>, typename SIMDType = xsimd::batch<NumericType>>
inline constexpr bool IsSIMDRegister = std::is_same_v<T, SIMDType>;
} // namespace chowdsp::SampleTypeHelpers
