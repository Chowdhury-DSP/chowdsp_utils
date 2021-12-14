#ifndef RTYPE_H_INCLUDED
#define RTYPE_H_INCLUDED

#include "wdf_t.h"
#include <array>

#if CHOWDSP_USE_XSIMD
#include <cassert>
#include <xsimd/xsimd.hpp>
#endif

namespace chowdsp::WDFT
{
#ifndef DOXYGEN
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
        ElementType& operator[] (int index) noexcept { return array[(size_t) index]; }
        const ElementType& operator[] (int index) const noexcept { return array[(size_t) index]; }

        auto begin() noexcept { return array.begin(); }
        auto begin() const noexcept { return array.begin(); }
        auto end() noexcept { return array.end(); }
        auto end() const noexcept { return array.end(); }
        auto data() noexcept { return array.data(); }
        auto data() const noexcept { return array.data(); }

        int size() const noexcept { return arraySize; }

        alignas (alignment) std::array<ElementType, (size_t) arraySize> array;
    };

    /** Implementation for float/double. */
    template <typename T, int numPorts>
    inline typename std::enable_if<std::is_same<float, T>::value || std::is_same<double, T>::value, void>::type
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
    template <typename T, int numPorts>
    inline typename std::enable_if<std::is_same<juce::dsp::SIMDRegister<float>, T>::value || std::is_same<juce::dsp::SIMDRegister<double>, T>::value, void>::type
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
} // namespace rtype_detail
#endif // DOXYGEN

/**
 *  A non-adaptable R-Type adaptor.
 *  For more information see: https://searchworks.stanford.edu/view/11891203, chapter 2
 */
template <typename T, typename... PortTypes>
class RootRtypeAdaptor : public RootWDF
{
public:
    /** Number of ports connected to RootRtypeAdaptor */
    static constexpr auto numPorts = sizeof...(PortTypes);

    explicit RootRtypeAdaptor (std::tuple<PortTypes&...> dps) : downPorts (dps)
    {
        for (int i = 0; i < (int) numPorts; i++)
        {
            b_vec[i] = (T) 0;
            a_vec[i] = (T) 0;
        }

        rtype_detail::forEachInTuple ([&] (auto& port, size_t)
                                      { port.connectToParent (this); },
                                      downPorts);
    }

    /**
     * If you are using this element in a WDF tree, you MUST implement
     * this method to update the scattering matrix.
     */
    std::function<void (const T (&)[numPorts])> calcImpedanceFunc = [] (const T (&)[numPorts]) {};

    /** Recomuptes internal variables based on the incoming impedances */
    void calcImpedance() override
    {
        T portImpedances[numPorts];
        rtype_detail::forEachInTuple ([&] (auto& port, size_t i)
                                      { portImpedances[i] = port.R; },
                                      downPorts);

        calcImpedanceFunc (portImpedances);
    }

    /** Use this function to set the scattering matrix data. */
    void setSMatrixData (const T (&mat)[numPorts][numPorts])
    {
        for (int i = 0; i < (int) numPorts; ++i)
            for (int j = 0; j < (int) numPorts; ++j)
                S_matrix[i][j] = mat[i][j];
    }

    /** Computes both the incident and reflected waves at this root node. */
    inline void compute() noexcept
    {
        rtype_detail::RtypeScatter (S_matrix, a_vec, b_vec);
        rtype_detail::forEachInTuple ([&] (auto& port, size_t i)
                                      {
                                          port.incident (b_vec[i]);
                                          a_vec[i] = port.reflected(); },
                                      downPorts);
    }

protected:
    std::tuple<PortTypes&...> downPorts; // tuple of ports connected to RtypeAdaptor

    rtype_detail::AlignedArray<T, numPorts> S_matrix[numPorts]; // square matrix representing S
    T a_vec alignas (16)[numPorts]; // temp matrix of inputs to Rport
    T b_vec alignas (16)[numPorts]; // temp matrix of outputs from Rport
};

/**
 *  An adaptable R-Type adaptor.
 *  For more information see: https://searchworks.stanford.edu/view/11891203, chapter 2
 */
template <typename T, int upPortIndex, typename... PortTypes>
class RtypeAdaptor : public BaseWDF
{
public:
    /** Number of ports connected to RtypeAdaptor */
    static constexpr auto numPorts = sizeof...(PortTypes) + 1;

    explicit RtypeAdaptor (std::tuple<PortTypes&...> dps) : downPorts (dps)
    {
        for (int i = 0; i < (int) numPorts; i++)
        {
            b_vec[i] = (T) 0;
            a_vec[i] = (T) 0;
        }

        rtype_detail::forEachInTuple ([&] (auto& port, size_t)
                                      { port.connectToParent (this); },
                                      downPorts);
    }

    /**
     * If you are using this element in a WDF tree, you MUST implement
     * this method to update the scattering matrix. The method should
     * return the new port impedance.
     */
    std::function<T (const T (&)[numPorts - 1])> calcImpedanceFunc = [] (const T (&)[numPorts - 1])
    { return (T) 1; };

    /** Re-computes the port impedance at the adapted upward-facing port */
    void calcImpedance() override
    {
        T portImpedances[numPorts - 1];
        rtype_detail::forEachInTuple ([&] (auto& port, size_t i)
                                      { portImpedances[i] = port.R; },
                                      downPorts);

        R = calcImpedanceFunc (portImpedances);
        G = (T) 1 / R;
    }

    /** Use this function to set the scattering matrix data. */
    void setSMatrixData (const T (&mat)[numPorts][numPorts])
    {
        for (int i = 0; i < (int) numPorts; ++i)
            for (int j = 0; j < (int) numPorts; ++j)
                S_matrix[i][j] = mat[i][j];
    }

    /** Computes the incident wave. */
    inline void incident (T downWave) noexcept
    {
        a = downWave;
        a_vec[upPortIndex] = a;

        rtype_detail::RtypeScatter (S_matrix, a_vec, b_vec);
        rtype_detail::forEachInTuple ([&] (auto& port, size_t i)
                                      {
                                          auto portIndex = getPortIndex ((int) i);
                                          port.incident (b_vec[portIndex]); },
                                      downPorts);
    }

    /** Computes the reflected wave */
    inline T reflected() noexcept
    {
        rtype_detail::forEachInTuple ([&] (auto& port, size_t i)
                                      {
                                          auto portIndex = getPortIndex ((int) i);
                                          a_vec[portIndex] = port.reflected(); },
                                      downPorts);

        b = b_vec[upPortIndex];
        return b;
    }

    using NumericType = typename SampleTypeHelpers::ElementType<T>::Type;
    T R = (NumericType) 1.0e-9; /* impedance */
    T G = (T) 1.0 / R; /* admittance */
    T a = (T) 0.0; /* incident wave */
    T b = (T) 0.0; /* reflected wave */

protected:
    constexpr auto getPortIndex (int tupleIndex)
    {
        return tupleIndex < upPortIndex ? tupleIndex : tupleIndex + 1;
    }

    std::tuple<PortTypes&...> downPorts; // tuple of ports connected to RtypeAdaptor

    rtype_detail::AlignedArray<T, numPorts> S_matrix[numPorts]; // square matrix representing S
    T a_vec alignas (16)[numPorts]; // temp matrix of inputs to Rport
    T b_vec alignas (16)[numPorts]; // temp matrix of outputs from Rport
};

} // namespace chowdsp::WDFT

#endif // RTYPE_H_INCLUDED
