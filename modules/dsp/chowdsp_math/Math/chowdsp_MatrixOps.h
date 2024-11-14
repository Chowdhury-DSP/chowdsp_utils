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
#if ! CHOWDSP_NO_XSIMD
        /** Perform out-of-place Householder transform (scalar types) */
        template <typename T = FloatType>
        static inline std::enable_if_t<std::is_floating_point_v<T>, void>
            outOfPlace (FloatType* out, const FloatType* in)
        {
            // must be used with aligned data!
            jassert (SIMDUtils::isAligned (in));
            jassert (SIMDUtils::isAligned (out));

            using Vec = xsimd::batch<T>;
            static constexpr auto vec_size = (int) Vec::size;
            static constexpr auto vec_loop_size = size - size % vec_size;

            Vec vecAccumulator {};
            int i = 0;
            for (; i < vec_loop_size; i += vec_size)
                vecAccumulator += xsimd::load_aligned (in + i);

            T scalarAccumulator = xsimd::reduce_add (vecAccumulator);
            if constexpr (size % vec_size != 0)
            {
                for (; i < size; ++i)
                    scalarAccumulator += in[i];
            }

            scalarAccumulator *= multiplier;

            int j = 0;
            for (; j < vec_loop_size; j += vec_size)
            {
                auto x = xsimd::load_aligned (in + j) + scalarAccumulator;
                xsimd::store_aligned (out + j, x);
            }

            if constexpr (size % vec_size != 0)
            {
                for (; j < size; ++j)
                    out[j] = in[j] + scalarAccumulator;
            }
        }

        /** Perform out-of-place Householder transform (SIMD types) */
        template <typename T = FloatType>
        static inline std::enable_if_t<SampleTypeHelpers::IsSIMDRegister<T>, void>
            outOfPlace (FloatType* out, const FloatType* in)
        {
            FloatType vecSum {};
            for (int i = 0; i < size; ++i)
                vecSum += in[i];

            const auto sum = multiplier * xsimd::reduce_add (vecSum);

            for (int i = 0; i < size; ++i)
                out[i] = in[i] + sum;
        }
#else
        /** Perform out-of-place Householder transform (scalar types) */
        template <typename T = FloatType>
        static inline std::enable_if_t<std::is_floating_point_v<T>, void>
            outOfPlace (FloatType* out, const FloatType* in)
        {
            FloatType sum {};
            for (int i = 0; i < size; ++i)
            {
                sum += in[i];
            }

            sum *= multiplier;

            for (int i = 0; i < size; ++i)
            {
                out[i] = in[i] + sum;
            }
        }
#endif

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
        static constexpr NumericType scalingFactor = gcem::sqrt ((NumericType) 1 / NumericType (size * SampleTypeHelpers::TypeTraits<FloatType>::Size));
        static_assert (Math::isPowerOfTwo (size * SampleTypeHelpers::TypeTraits<FloatType>::Size), "Hadamard matrix dimension must be a power of 2!");

    public:
        /** Perform unscaled Hadamard transformation using recursion */
        template <typename T = FloatType>
        static inline std::enable_if_t<std::is_floating_point_v<T> || (size > 1), void>
            recursiveUnscaled (FloatType* out, const FloatType* in)
        {
            static constexpr int hSize = size / 2;

            if constexpr (size == 2) // base case
            {
                if constexpr (SampleTypeHelpers::IsSIMDRegister<T>)
                {
                    Hadamard<FloatType, hSize>::recursiveUnscaled (out, in);
                    Hadamard<FloatType, hSize>::recursiveUnscaled (out + hSize, in + hSize);
                }

                const auto a_plus_b = in[0] + in[1];
                const auto a_minus_b = in[0] - in[1];
                out[0] = a_plus_b;
                out[1] = a_minus_b;
            }
            else
            {
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

#if ! CHOWDSP_NO_XSIMD
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

        /** Perform in-place Hadamard transformation (scalar types) */
        template <typename T = FloatType>
        static inline std::enable_if_t<std::is_floating_point_v<T>, void>
            inPlace (FloatType* arr)
        {
            // must be used with aligned data!
            jassert (SIMDUtils::isAligned (arr));

            using Vec = xsimd::batch<T>;
            static constexpr auto vec_size = (int) Vec::size;

            if constexpr (size <= vec_size)
            {
                recursiveUnscaled (arr, arr);

                for (int i = 0; i < size; ++i)
                    arr[i] *= scalingFactor;
            }
            else
            {
                Vec arrVec[size_t (size / vec_size)] {};
                for (int i = 0; i < size; i += vec_size)
                    arrVec[i / vec_size] = xsimd::load_aligned (arr + i);

                Hadamard<Vec, size / vec_size>::recursiveUnscaled (arrVec, arrVec);

                // multiply by scaling factor
                for (int i = 0; i < size; i += vec_size)
                    xsimd::store_aligned (arr + i, arrVec[i / vec_size] * scalingFactor);
            }
        }

        /** Perform in-place Hadamard transformation (SIMD types) */
        template <typename T = FloatType>
        static inline std::enable_if_t<SampleTypeHelpers::IsSIMDRegister<T>, void>
            inPlace (FloatType* arr)
        {
            recursiveUnscaled (arr, arr);

            for (int i = 0; i < size; ++i)
                arr[i] *= scalingFactor;
        }
#else
        template <typename T = FloatType>
        static inline std::enable_if_t<std::is_floating_point_v<T>, void>
            inPlace (FloatType* arr)
        {
            recursiveUnscaled (arr, arr);

            for (int i = 0; i < size; ++i)
                arr[i] *= scalingFactor;
        }
#endif
    };
} // namespace MatrixOps
} // namespace chowdsp
