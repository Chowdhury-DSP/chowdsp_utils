#ifndef RTYPE_H_INCLUDED
#define RTYPE_H_INCLUDED

namespace chowdsp
{
namespace WDF
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
    } // namespace rtype_detail

    /**
 *  A non-adaptable R-Type adaptor.
 *  For more information see: https://searchworks.stanford.edu/view/11891203, chapter 2
 */
    template <typename T, typename... PortTypes>
    class RootRtypeAdaptor : public WDFNode<T>
    {
        static constexpr int numPorts = sizeof...(PortTypes); // number of ports connected to RtypeAdaptor
    public:
        RootRtypeAdaptor (std::tuple<PortTypes&...> dps) : WDFNode<T> ("Root R-type Adaptor"), downPorts (dps)
        {
            for (int i = 0; i < numPorts; i++)
            {
                b_vec[i] = (T) 0;
                a_vec[i] = (T) 0;
            }

            rtype_detail::forEachInTuple ([&] (auto& port, size_t) { port.connectToNode (this); }, downPorts);
        }

        void calcImpedance() override {}

        /** Use this function to set the scattering matrix data. */
        void setSMatrixData (const T (&mat)[numPorts][numPorts])
        {
            for (int i = 0; i < numPorts; ++i)
                for (int j = 0; j < numPorts; ++j)
                    S_matrix[i][j] = mat[i][j];
        }

        /** Computes the incident wave. Note that the input value is not used. */
        void incident (T /*downWave*/) noexcept override
        {
            RtypeScatter (S_matrix, a_vec, b_vec);
            rtype_detail::forEachInTuple ([&] (auto& port, size_t i) {
                port.incident (b_vec[i]);
                a_vec[i] = port.reflected();
            },
                                          downPorts);
        }

    protected:
        /** Implementation for float/double. */
        template <typename C = T>
        static inline typename std::enable_if<std::is_same<float, C>::value || std::is_same<double, C>::value, void>::type
            RtypeScatter (const T (&S_)[numPorts][numPorts], const T (&a_)[numPorts], T (&b_)[numPorts])
        {
            // input matrix (S) of size dim x dim
            // input vector (a) of size 1 x dim
            // output vector (b) of size 1 x dim

#if CHOWDSP_USE_XSIMD
            using v_type = xsimd::simd_type<T>;
            constexpr auto simd_size = (int) v_type::size;
            constexpr auto vec_size = numPorts - numPorts % simd_size;

            for (int c = 0; c < numPorts; ++c)
            {
                v_type bc { (T) 0 };
                for (int r = 0; r < vec_size; r += simd_size)
                    bc = xsimd::fma(xsimd::load_aligned (S_[c] + r), xsimd::load_aligned (a_ + r), bc);
                b_[c] = xsimd::hadd (bc);

                // remainder of ops that can't be vectorized
                for (int r = vec_size; r < numPorts; ++r)
                    b_[c] += S_[c][r] * a_[r];
            }
#elif JUCE_USE_SIMD
            using v_type = juce::dsp::SIMDRegister<T>;
            constexpr auto simd_size = v_type::size();
            constexpr auto vec_size = numPorts - numPorts % simd_size;

            for (int c = 0; c < numPorts; ++c)
            {
                b_[c] = (T) 0;
                for (int r = 0; r < vec_size; r += simd_size)
                    b_[c] += (v_type::fromRawArray (S_[c] + r) * v_type::fromRawArray (a_ + r)).sum();

                // remainder of ops that can't be vectorized
                for (int r = vec_size; r < numPorts; ++r)
                    b_[c] += S_[c][r] * a_[r];
            }
#else // No SIMD
            for (int c = 0; c < numPorts; ++c)
            {
                b_[c] = (T) 0;
                for (int r = 0; r < numPorts; ++r)
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
            RtypeScatter (const T (&S_)[numPorts][numPorts], const T (&a_)[numPorts], T (&b_)[numPorts])
        {
            for (int c = 0; c < numPorts; ++c)
            {
                b_[c] = (T) 0;
                for (int r = 0; r < numPorts; ++r)
                    b_[c] += S_[c][r] * a_[r];
            }
        }
#endif // USING JUCE

        std::tuple<PortTypes&...> downPorts; // tuple of ports connected to RtypeAdaptor

        T S_matrix alignas (16)[numPorts][numPorts]; // square matrix representing S
        T a_vec alignas (16)[numPorts]; // temp matrix of inputs to Rport
        T b_vec alignas (16)[numPorts]; // temp matrix of outputs from Rport

    private:
        /** Computes the reflected wave (no-op) */
        T reflected() noexcept override
        {
            return 0.0;
        }
    };

} // namespace WDF
} // namespace chowdsp

#endif // RTYPE_H_INCLUDED
