#pragma once

#include <type_traits>

namespace chowdsp
{
/** Useful matrix operations */
namespace MatrixOps
{
#ifndef DOXYGEN
    namespace detail
    {
        template <typename T>
        static bool isAligned (const T* p) noexcept
        {
            static constexpr auto RegisterSize = sizeof (xsimd::batch<T>);
            uintptr_t bitmask = RegisterSize - 1;
            return ((uintptr_t) p & bitmask) == 0;
        }

        constexpr bool isPowerOfTwo (int n)
        {
            return (n & (n - 1)) == 0;
        }
    } // namespace detail
#endif

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
            // must be used with aligned data!
            jassert (detail::isAligned (in));
            jassert (detail::isAligned (out));

            using Vec = xsimd::batch<T>;
            static constexpr auto vec_size = (int) Vec::size;
            static constexpr auto vec_loop_size = size - size % vec_size;

            Vec vecAccumulator {};
            int i = 0;
            for (; i < vec_loop_size; i += vec_size)
                vecAccumulator += xsimd::load_aligned (in + i);

            T scalarAccumulator = xsimd::hadd (vecAccumulator);
            if constexpr (size % vec_size != 0)
            {
                for (; i < size; ++i)
                    scalarAccumulator += in[i];
            }

            static constexpr auto normMultiplier = (T) -2 / (T) size;
            scalarAccumulator *= normMultiplier;

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
        static constexpr NumericType scalingFactor = gcem::sqrt ((NumericType) 1 / NumericType (size * SampleTypeHelpers::TypeTraits<FloatType>::Size));
        static_assert (detail::isPowerOfTwo (size * SampleTypeHelpers::TypeTraits<FloatType>::Size), "Hadamard matrix dimension must be a power of 2!");

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
                static constexpr int hSize = size / 2;

                // Two (unscaled) Hadamards of half the size
                Hadamard<FloatType, hSize>::recursiveUnscaled (out, in);
                Hadamard<FloatType, hSize>::recursiveUnscaled (out + hSize, in + hSize);

                // Combine the two halves using sum/difference
                if constexpr (SampleTypeHelpers::IsSIMDRegister<T>)
                {
                    for (int i = 0; i < hSize; ++i)
                    {
                        FloatType a = in[i];
                        FloatType b = in[i + hSize];
                        out[i] = a + b;
                        out[i + hSize] = a - b;
                    }
                }
                else if constexpr (hSize < (int) xsimd::batch<T>::size)
                {
                    for (int i = 0; i < hSize; ++i)
                    {
                        FloatType a = in[i];
                        FloatType b = in[i + hSize];
                        out[i] = a + b;
                        out[i + hSize] = a - b;
                    }
                }
                else
                {
                    using Vec = xsimd::batch<T>;
                    static constexpr auto vec_size = (int) Vec::size;
                    static constexpr auto vec_loop_hsize = hSize - hSize % vec_size;

                    int i = 0;
                    for (; i < vec_loop_hsize; i += vec_size)
                    {
                        const auto a = xsimd::load_aligned (in + i);
                        const auto b = xsimd::load_aligned (in + hSize + i);

                        xsimd::store_aligned (out + i, a + b);
                        xsimd::store_aligned (out + hSize + i, a - b);
                    }
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
            inPlace (FloatType* arr)
        {
            // must be used with aligned data!
            jassert (detail::isAligned (arr));

            recursiveUnscaled (arr, arr);

            // multiply by scaling factor
            using Vec = xsimd::batch<T>;
            static constexpr auto vec_size = (int) Vec::size;
            static constexpr auto vec_loop_size = size - size % vec_size;

            int i = 0;
            for (; i < vec_loop_size; i += vec_size)
            {
                auto x = xsimd::load_aligned (arr + i) * scalingFactor;
                xsimd::store_aligned (arr + i, x);
            }
        }

        /** Perform out-of-place Hadamard transformation (SIMD types) */
        template <typename T = FloatType>
        static inline std::enable_if_t<SampleTypeHelpers::IsSIMDRegister<T>, void>
            inPlace (FloatType* arr)
        {
            recursiveUnscaled (arr, arr);

            for (int i = 0; i < size; ++i)
                arr[i] *= scalingFactor;
        }
    };
} // namespace MatrixOps

} // namespace chowdsp
