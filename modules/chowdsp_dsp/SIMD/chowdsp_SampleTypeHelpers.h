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
struct ElementType<T, false>
{
    using Type = typename T::value_type;
    static constexpr int Size = (int) T::size();
};

/** Type alias for a SIMD numeric type */
template <typename SampleType>
using NumericType = typename ElementType<SampleType>::Type;

/** Type alias for retrieving a SIMD numeric type for a processor that has one defined */
template <typename ProcessorType>
using ProcessorNumericType = typename ProcessorType::NumericType;

/** Useful template expression for determining if a type is a SIMDRegister */
template <typename T, typename NumericType = NumericType<T>, typename SIMDType = juce::dsp::SIMDRegister<NumericType>>
inline constexpr bool IsSIMDRegister = std::is_same_v<T, SIMDType>;

/** Type alias for the vector mask type of a SIMD register */
template <typename SIMDType, typename = std::enable_if_t<IsSIMDRegister<SIMDType>>>
using vMaskType = typename SIMDType::vMaskType;

/** Type alias for the vector mask type of a SIMD register with the given BaseType */
template <typename BaseType, typename = std::enable_if_t<std::is_floating_point_v<BaseType>>>
using vMaskTypeSIMD = vMaskType<juce::dsp::SIMDRegister<BaseType>>;
} // namespace chowdsp::SampleTypeHelpers
