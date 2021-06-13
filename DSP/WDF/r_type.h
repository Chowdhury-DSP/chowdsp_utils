#ifndef RTYPE_H_INCLUDED
#define RTYPE_H_INCLUDED

#include "wdf_t.h"
#include <array>

#if CHOWDSP_USE_XSIMD
#include <cassert>
#include <xsimd/xsimd.hpp>
#endif

namespace chowdsp
{
namespace WDFT
{
    /** Utility functions used internally by the R-Type adaptor */
    namespace rtype_detail
    {
        /** Functions to do a function for each element in the tuple */
        template <typename Fn, typename Tuple, size_t... Ix>
        constexpr void forEachInTuple (Fn&& fn, Tuple&& tuple, std::index_sequence<Ix...>) noexcept (noexcept (std::initializer_list<int> { (fn (std::get<Ix> (tuple), Ix), 0)... }))
        {
            (void) std::initializer_list<int> { ((void) fn (std::get<Ix> (tuple), Ix), 0)... };
        }

        template <typename T>
        using TupleIndexSequence = std::make_index_sequence<std::tuple_size<std::remove_cv_t<std::remove_reference_t<T>>>::value>;

        template <typename Fn, typename Tuple>
        constexpr void forEachInTuple (Fn&& fn, Tuple&& tuple) noexcept (noexcept (forEachInTuple (std::forward<Fn> (fn), std::forward<Tuple> (tuple), TupleIndexSequence<Tuple> {})))
        {
            forEachInTuple (std::forward<Fn> (fn), std::forward<Tuple> (tuple), TupleIndexSequence<Tuple> {});
        }

        template <typename ElementType, int arraySize, int alignment = 16>
        struct AlignedArray
        {
            ElementType& operator[] (int index) noexcept { return array[(int) index]; }
            const ElementType& operator[] (int index) const noexcept { return array[(int) index]; }

            auto begin() noexcept { return array.begin(); }
            auto begin() const noexcept { return array.begin(); }
            auto end() noexcept { return array.end(); }
            auto end() const noexcept { return array.end(); }
            auto data() noexcept { return array.data(); }
            auto data() const noexcept { return array.data(); }

            int size() const noexcept { return arraySize; }

            alignas (alignment) std::array<ElementType, arraySize> array;
        };
    } // namespace rtype_detail

    /**
 *  A non-adaptable R-Type adaptor.
 *  For more information see: https://searchworks.stanford.edu/view/11891203, chapter 2
 */
    template <typename T, typename... PortTypes>
    class RootRtypeAdaptor
    {
        static constexpr auto numPorts = sizeof...(PortTypes); // number of ports connected to RtypeAdaptor
    public:
        RootRtypeAdaptor (std::tuple<PortTypes&...> dps) : downPorts (dps)
        {
            for (int i = 0; i < (int) numPorts; i++)
            {
                b_vec[i] = (T) 0;
                a_vec[i] = (T) 0;
            }

            // @TODO: I don't think this is necessary since this is the root?
            // rtype_detail::forEachInTuple ([&] (auto& port, size_t) { port.connectToParent (this); }, downPorts);
        }

        /** Use this function to set the scattering matrix data. */
        void setSMatrixData (const T (&mat)[numPorts][numPorts])
        {
            for (int i = 0; i < (int) numPorts; ++i)
                for (int j = 0; j < (int) numPorts; ++j)
                    S_matrix[i][j] = mat[i][j];
        }

        /** Computes the incident wave. Note that the input value is not used. */
        inline void incident (T /*downWave*/) noexcept
        {
            RtypeScatter (S_matrix, a_vec, b_vec);
            rtype_detail::forEachInTuple ([&] (auto& port, size_t i) {
                port.incident (b_vec[i]);
                a_vec[i] = port.reflected();
            },
                                          downPorts);
        }

        template <int port_idx>
        constexpr auto getPort()
        {
            return std::get<port_idx> (downPorts);
        }

    protected:
        /** Implementation for float/double. */
        template <typename C = T>
        static inline typename std::enable_if<std::is_same<float, C>::value || std::is_same<double, C>::value, void>::type
            RtypeScatter (const rtype_detail::AlignedArray<T, numPorts> (&S_)[numPorts], const T (&a_)[numPorts], T (&b_)[numPorts])
        {
            // input matrix (S) of size dim x dim
            // input vector (a) of size 1 x dim
            // output vector (b) of size 1 x dim

#if CHOWDSP_USE_XSIMD
            using v_type = xsimd::simd_type<T>;
            constexpr auto simd_size = (int) v_type::size;
            constexpr auto vec_size = (int) numPorts - (int) numPorts % simd_size;

            for (int c = 0; c < (int) numPorts; ++c)
            {
                v_type bc { (T) 0 };
                for (int r = 0; r < vec_size; r += simd_size)
                    bc = xsimd::fma (xsimd::load_aligned (S_[c].data() + r), xsimd::load_aligned (a_ + r), bc);
                b_[c] = xsimd::hadd (bc);

                // remainder of ops that can't be vectorized
                for (int r = vec_size; r < (int) numPorts; ++r)
                    b_[c] += S_[c][r] * a_[r];
            }
#elif JUCE_USE_SIMD
            using v_type = juce::dsp::SIMDRegister<T>;
            constexpr auto simd_size = (int) v_type::size();
            constexpr auto vec_size = (int) numPorts - (int) numPorts % simd_size;

            for (int c = 0; c < (int) numPorts; ++c)
            {
                b_[c] = (T) 0;
                for (int r = 0; r < vec_size; r += simd_size)
                    b_[c] += (v_type::fromRawArray (S_[c].data() + r) * v_type::fromRawArray (a_ + r)).sum();

                // remainder of ops that can't be vectorized
                for (int r = vec_size; r < (int) numPorts; ++r)
                    b_[c] += S_[c][r] * a_[r];
            }
#else // No SIMD
            for (int c = 0; c < (int) numPorts; ++c)
            {
                b_[c] = (T) 0;
                for (int r = 0; r < (int) numPorts; ++r)
                    b_[c] += S_[c][r] * a_[r];
            }
#endif // SIMD options
        }

#if USING_JUCE
        /** Implementation for SIMD float/double. */
        template <typename C = T>
        static inline typename std::enable_if<std::is_same<juce::dsp::SIMDRegister<float>, C>::value
                                                  || std::is_same<juce::dsp::SIMDRegister<double>, C>::value,
                                              void>::type
            RtypeScatter (const rtype_detail::AlignedArray<T, numPorts> (&S_)[numPorts], const T (&a_)[numPorts], T (&b_)[numPorts])
        {
            for (int c = 0; c < (int) numPorts; ++c)
            {
                b_[c] = (T) 0;
                for (int r = 0; r < (int) numPorts; ++r)
                    b_[c] += S_[c][r] * a_[r];
            }
        }
#endif // USING JUCE

        std::tuple<PortTypes&...> downPorts; // tuple of ports connected to RtypeAdaptor

        rtype_detail::AlignedArray<T, numPorts> S_matrix[numPorts]; // square matrix representing S
        T a_vec alignas (16)[numPorts]; // temp matrix of inputs to Rport
        T b_vec alignas (16)[numPorts]; // temp matrix of outputs from Rport
    };

} // namespace WDFT
} // namespace chowdsp

#endif // RTYPE_H_INCLUDED
