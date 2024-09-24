#pragma once

namespace chowdsp
{
/**
    A collection of structs to pass as the template argument when setting the
    interpolation type for the DelayLine class.
*/
namespace DelayLineInterpolationTypes
{
    /**
    No interpolation between successive samples in the delay line will be
    performed. This is useful when the delay is a constant integer or to
    create lo-fi audio effects.
*/
    struct None
    {
        template <typename T>
        void updateInternalVariables (int& /*delayIntOffset*/, T& /*delayFrac*/) // NOSONAR (template compatibility)
        {
        }

        template <typename SampleType, typename NumericType = SampleTypeHelpers::NumericType<SampleType>, typename StorageType = SampleType>
        inline SampleType call (const SampleType* buffer, int delayInt, NumericType /*delayFrac*/ = {}, const SampleType& /*state*/ = {})
        {
            return static_cast<SampleType> (buffer[delayInt]);
        }
    };

    /**
    Successive samples in the delay line will be linearly interpolated. This
    type of interpolation has a low compuational cost where the delay can be
    modulated in real time, but it also introduces a low-pass filtering effect
    into your audio signal.
*/
    struct Linear
    {
        template <typename T>
        void updateInternalVariables (int& /*delayIntOffset*/, T& /*delayFrac*/) // NOSONAR (template compatibility)
        {
        }

        template <typename SampleType, typename NumericType, typename StorageType = SampleType>
        inline SampleType call (const StorageType* buffer, int delayInt, NumericType delayFrac, const SampleType& /*state*/ = {})
        {
            auto index1 = delayInt;
            auto index2 = index1 + 1;

            auto value1 = static_cast<SampleType> (buffer[index1]);
            auto value2 = static_cast<SampleType> (buffer[index2]);

            return value1 + (SampleType) delayFrac * (value2 - value1);
        }
    };

    /**
    Successive samples in the delay line will be interpolated using a 3rd order
    Lagrange interpolator. This method incurs more computational overhead than
    linear interpolation but reduces the low-pass filtering effect whilst
    remaining amenable to real time delay modulation.
*/
    struct Lagrange3rd
    {
        template <typename T>
        void updateInternalVariables (int& delayIntOffset, T& delayFrac) // NOSONAR (template compatibility)
        {
            if (delayIntOffset >= 1)
            {
                delayFrac++;
                delayIntOffset--;
            }
        }

        template <typename SampleType, typename NumericType, typename StorageType = SampleType>
        inline SampleType call (const StorageType* buffer, int delayInt, NumericType delayFrac, const SampleType& /*state*/ = {})
        {
            auto index1 = delayInt;
            auto index2 = index1 + 1;
            auto index3 = index2 + 1;
            auto index4 = index3 + 1;

            auto value1 = static_cast<SampleType> (buffer[index1]);
            auto value2 = static_cast<SampleType> (buffer[index2]);
            auto value3 = static_cast<SampleType> (buffer[index3]);
            auto value4 = static_cast<SampleType> (buffer[index4]);

            auto d1 = delayFrac - (NumericType) 1.0;
            auto d2 = delayFrac - (NumericType) 2.0;
            auto d3 = delayFrac - (NumericType) 3.0;

            auto c1 = -d1 * d2 * d3 / (NumericType) 6.0;
            auto c2 = d2 * d3 * (NumericType) 0.5;
            auto c3 = -d1 * d3 * (NumericType) 0.5;
            auto c4 = d1 * d2 / (NumericType) 6.0;

            return value1 * c1 + (SampleType) delayFrac * (value2 * c2 + value3 * c3 + value4 * c4);
        }
    };

    /**
    Successive samples in the delay line will be interpolated using a 5th order
    Lagrange interpolator. This method incurs more computational overhead than
    linear interpolation.
*/
    struct Lagrange5th
    {
        template <typename T>
        void updateInternalVariables (int& delayIntOffset, T& delayFrac) // NOSONAR (template compatibility)
        {
            if (delayIntOffset >= 2)
            {
                delayFrac += (T) 2;
                delayIntOffset -= 2;
            }
        }

        template <typename SampleType, typename NumericType, typename StorageType = SampleType>
        inline SampleType call (const StorageType* buffer, int delayInt, NumericType delayFrac, const SampleType& /*state*/ = {})
        {
            auto index1 = delayInt;
            auto index2 = index1 + 1;
            auto index3 = index2 + 1;
            auto index4 = index3 + 1;
            auto index5 = index4 + 1;
            auto index6 = index5 + 1;

            auto value1 = static_cast<SampleType> (buffer[index1]);
            auto value2 = static_cast<SampleType> (buffer[index2]);
            auto value3 = static_cast<SampleType> (buffer[index3]);
            auto value4 = static_cast<SampleType> (buffer[index4]);
            auto value5 = static_cast<SampleType> (buffer[index5]);
            auto value6 = static_cast<SampleType> (buffer[index6]);

            auto d1 = delayFrac - (NumericType) 1.0;
            auto d2 = delayFrac - (NumericType) 2.0;
            auto d3 = delayFrac - (NumericType) 3.0;
            auto d4 = delayFrac - (NumericType) 4.0;
            auto d5 = delayFrac - (NumericType) 5.0;

            auto c1 = -d1 * d2 * d3 * d4 * d5 / (NumericType) 120.0;
            auto c2 = d2 * d3 * d4 * d5 / (NumericType) 24.0;
            auto c3 = -d1 * d3 * d4 * d5 / (NumericType) 12.0;
            auto c4 = d1 * d2 * d4 * d5 / (NumericType) 12.0;
            auto c5 = -d1 * d2 * d3 * d5 / (NumericType) 24.0;
            auto c6 = d1 * d2 * d3 * d4 / (NumericType) 120.0;

            return value1 * c1 + (SampleType) delayFrac * (value2 * c2 + value3 * c3 + value4 * c4 + value5 * c5 + value6 * c6);
        }
    };

    /**
    Successive samples in the delay line will be interpolated using 1st order
    Thiran interpolation. This method is very efficient, and features a flat
    amplitude frequency response in exchange for less accuracy in the phase
    response. This interpolation method is stateful so is unsuitable for
    applications requiring fast delay modulation.
*/
    struct Thiran
    {
        template <typename T>
        void updateInternalVariables (int& delayIntOffset, T& delayFrac)
        {
            if (delayFrac < (T) 0.618 && delayIntOffset >= 1)
            {
                delayFrac++;
                delayIntOffset--;
            }

            alpha = double ((1 - delayFrac) / (1 + delayFrac));
        }

        template <typename T1, typename T2>
        inline T1 call (const T1* buffer, int delayInt, T2 /*delayFrac*/, T1& state)
        {
            auto index1 = delayInt;
            auto index2 = index1 + 1;

            auto value1 = buffer[index1];
            auto value2 = buffer[index2];

            auto output = value2 + (T1) (T2) alpha * (value1 - state);
            state = output;

            return output;
        }

        double alpha = 0.0;
    };

#ifndef DOXYGEN
    JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4324) // MSVC doesn't like other variables hiding class members
#endif

    /**
    Successive samples in the delay line will be interpolated using Sinc
    interpolation. This method is somewhat less efficient than the others,
    but gives a very smooth and flat frequency response.

    Note that Sinc interpolation cannot currently be used with SIMD data types!
*/
    template <typename T, size_t N, size_t M = 256>
    struct Sinc
    {
        Sinc()
        {
            T cutoff = 0.455f;
            size_t j;
            for (j = 0; j < M + 1; j++)
            {
                for (size_t i = 0; i < N; i++)
                {
                    T t = -T (i) + T (N / (T) 2.0) + T (j) / T (M) - (T) 1.0;
                    sinctable[j * N * 2 + i] = symmetric_blackman (t, (int) N) * cutoff * sincf (cutoff * t);
                }
            }
            for (j = 0; j < M; j++)
            {
                for (size_t i = 0; i < N; i++)
                    sinctable[j * N * 2 + N + i] = (sinctable[(j + 1) * N * 2 + i] - sinctable[j * N * 2 + i]) / (T) 65536.0;
            }
        }

        inline T sincf (T x) const noexcept
        {
            if (x == (T) 0)
                return (T) 1;
            return (std::sin (juce::MathConstants<T>::pi * x)) / (juce::MathConstants<T>::pi * x);
        }

        inline T symmetric_blackman (T i, int n) const noexcept
        {
            i -= (n / 2);
            return ((T) 0.42 - (T) 0.5 * std::cos (juce::MathConstants<T>::twoPi * i / (n))
                    + (T) 0.08 * std::cos (4 * juce::MathConstants<T>::pi * i / (n)));
        }

        void updateInternalVariables (int& /*delayIntOffset*/, T& /*delayFrac*/) {} // NOSONAR (template compatibility)

        inline T call (const T* buffer, int delayInt, T delayFrac, const T& /*state*/ = {})
        {
            const auto sincTableOffset = (size_t) (((T) 1 - delayFrac) * (T) M) * N * 2;

#if CHOWDSP_NO_XSIMD
            auto out = (T) 0;
            for (size_t i = 0; i < N; ++i)
                out += buffer[(size_t) delayInt + i] * sinctable[sincTableOffset + i];
            return out;
#else
            auto out = xsimd::batch<T> ((T) 0);
            for (size_t i = 0; i < N; i += xsimd::batch<T>::size)
            {
                auto buff_reg = xsimd::load_unaligned (&buffer[(size_t) delayInt + i]);
                auto sinc_reg = xsimd::load_aligned (&sinctable[sincTableOffset + i]);
                out += buff_reg * sinc_reg;
            }

            return xsimd::reduce_add (out);
#endif
        }

        T sinctable alignas (SIMDUtils::defaultSIMDAlignment)[(M + 1) * N * 2];
    };
    JUCE_END_IGNORE_WARNINGS_MSVC
} // namespace DelayLineInterpolationTypes
} // namespace chowdsp
