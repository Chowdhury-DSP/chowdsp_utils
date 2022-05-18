#pragma once

#include <type_traits>

namespace chowdsp
{
/** Useful matrix operations */
namespace MatrixOps
{
    /**
     * Methods for implementing a Householder mixing matrix.
     * Inspired by: https://github.com/Signalsmith-Audio/reverb-example-code/blob/main/mix-matrix.h
     */
    template <typename FloatType, int size>
    struct HouseHolder
    {
    private:
        using NumericType = SampleTypeHelpers::NumericType<FloatType>;
        static constexpr NumericType multiplier = (NumericType) -2 / (NumericType) (size * SampleTypeHelpers::TypeTraits<FloatType>::Size);

    public:
        /** Perform out-of-place Householder transform (scalar types) */
        template <typename T = FloatType>
        static inline std::enable_if_t<std::is_floating_point_v<T>, void>
            outOfPlace (FloatType* out, const FloatType* in)
        {
            const auto sum = FloatVectorOperations::accumulate (in, size);
            juce::FloatVectorOperations::add (out, in, sum * multiplier, size);
        }

        /** Perform out-of-place Householder transform (SIMD types) */
        template <typename T = FloatType>
        static inline std::enable_if_t<SampleTypeHelpers::IsSIMDRegister<T>, void>
            outOfPlace (FloatType* out, const FloatType* in)
        {
            NumericType sum = 0;
            for (int i = 0; i < size; ++i)
                sum += xsimd::hadd (in[i]);

            sum *= multiplier;

            for (int i = 0; i < size; ++i)
                out[i] += sum;
        }

        /** Perform in-place Householder transform */
        static inline void inPlace (FloatType* arr)
        {
            outOfPlace (arr, arr);
        }
    };

    /**
     * Methods for implementing a Householder mixing matrix.
     * Inspired by: https://github.com/Signalsmith-Audio/reverb-example-code/blob/main/mix-matrix.h
     */
    template <typename FloatType, int size>
    struct Hadamard
    {
    private:
        using NumericType = SampleTypeHelpers::NumericType<FloatType>;
        static const NumericType scalingFactor;

    public:
        /** Perform unscaled Hadamard transformation using recursion */
        template <typename T = FloatType>
        static inline std::enable_if_t<std::is_floating_point_v<T> || (size > 1), void>
            recursiveUnscaled (FloatType* out, const FloatType* in)
        {
            if constexpr (size <= 1)
            {
                juce::ignoreUnused (out, in);
                return;
            }
            else
            {
                constexpr int hSize = size / 2;

                // Two (unscaled) Hadamards of half the size
                Hadamard<FloatType, hSize>::recursiveUnscaled (out, in);
                Hadamard<FloatType, hSize>::recursiveUnscaled (out + hSize, in + hSize);

                // Combine the two halves using sum/difference
                for (int i = 0; i < hSize; ++i)
                {
                    FloatType a = in[i];
                    FloatType b = in[i + hSize];
                    out[i] = a + b;
                    out[i + hSize] = a - b;
                }
            }
        }

        /** Perform unscaled Hadamard transformation (SIMD fallback) */
        template <typename T = FloatType>
        static inline std::enable_if_t<SampleTypeHelpers::IsSIMDRegister<T> && size == 1, void>
            recursiveUnscaled (FloatType* out, const FloatType* in)
        {
            constexpr auto VecSize = FloatType::size;
            NumericType arr alignas (xsimd::default_arch::alignment())[VecSize];

            xsimd::store_aligned (arr, in[0]);
            Hadamard<NumericType, VecSize>::recursiveUnscaled (arr, arr);
            out[0] = xsimd::load_aligned (arr);
        }

        /** Perform out-of-place Hadamard transformation (scalar types) */
        template <typename T = FloatType>
        static inline std::enable_if_t<std::is_floating_point_v<T>, void>
            outOfPlace (FloatType* out, const FloatType* in)
        {
            recursiveUnscaled (out, in);
            juce::FloatVectorOperations::multiply (out, scalingFactor, size);
        }

        /** Perform out-of-place Hadamard transformation (SIMD types) */
        template <typename T = FloatType>
        static inline std::enable_if_t<SampleTypeHelpers::IsSIMDRegister<T>, void>
            outOfPlace (FloatType* out, const FloatType* in)
        {
            recursiveUnscaled (out, in);

            for (int i = 0; i < size; ++i)
                out[i] *= scalingFactor;
        }

        /** Perform in-place Hadamard transformation */
        static inline void inPlace (FloatType* arr)
        {
            outOfPlace (arr, arr);
        }
    };

    template <typename FloatType, int size>
    const SampleTypeHelpers::NumericType<FloatType> Hadamard<FloatType, size>::scalingFactor = std::sqrt ((NumericType) 1 / NumericType (size * SampleTypeHelpers::TypeTraits<FloatType>::Size));
} // namespace MatrixOps

} // namespace chowdsp
