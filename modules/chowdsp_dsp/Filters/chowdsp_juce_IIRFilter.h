/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

/**
    Classes for IIR filter processing.

    Ported from JUCE DSP IIR, but uses static order filters,
    determined at compile-time
*/
namespace chowdsp::IIR
{
template <typename NumericType, size_t order = 2>
struct Coefficients;

/**
    A processing class that can perform IIR filtering on an audio signal, using
    the Transposed Direct Form II digital structure.

    This has been modified from the original JUCE implementation to use template
    arguments to determine filter order at compile-time.

    If you need a lowpass, bandpass or highpass filter with fast modulation of
    its cutoff frequency, you might use the class StateVariableFilter instead,
    which is designed to prevent artefacts at parameter changes, instead of the
    class Filter.

    @see Filter::Coefficients, FilterAudioSource, StateVariableFilter
*/
template <typename SampleType, size_t order = 2>
class Filter
{
public:
    /** The NumericType is the underlying primitive type used by the SampleType (which
        could be either a primitive or vector)
    */
    using NumericType = typename juce::dsp::SampleTypeHelpers::ElementType<SampleType>::Type;

    /** A typedef for a ref-counted pointer to the coefficients object */
    using CoefficientsPtr = typename Coefficients<NumericType, order>::Ptr;

    //==============================================================================
    /** Creates a filter.

        Initially the filter is inactive, so will have no effect on samples that
        you process with it. You can modify the coefficients member to turn it into
        the type of filter needed.
    */
    Filter();

    /** Creates a filter with a given set of coefficients. */
    explicit Filter (CoefficientsPtr coefficientsToUse);

    Filter (const Filter&) = default;
    Filter (Filter&&) noexcept = default;
    Filter& operator= (const Filter&) = default;
    Filter& operator= (Filter&&) noexcept = default;

    //==============================================================================
    /** The coefficients of the IIR filter. It's up to the caller to ensure that
        these coefficients are modified in a thread-safe way.

        If you change the order of the coefficients then you must call reset after
        modifying them.
    */
    CoefficientsPtr coefficients;

    //==============================================================================
    /** Resets the filter's processing pipeline, ready to start a new stream of data.

        Note that this clears the processing state, but the type of filter and
        its coefficients aren't changed.
    */
    void reset() { reset (SampleType { 0 }); }

    /** Resets the filter's processing pipeline to a specific value.
        @see reset
    */
    void reset (SampleType resetToValue);

    //==============================================================================
    /** Called before processing starts. */
    void prepare (const juce::dsp::ProcessSpec&) noexcept;

    /** Processes a block of samples */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        if (context.isBypassed)
            processInternal<ProcessContext, true> (context);
        else
            processInternal<ProcessContext, false> (context);

#if JUCE_SNAP_TO_ZERO
        snapToZero();
#endif
    }

    /** Processes a single sample, without any locking.

        Use this if you need processing of a single value.

        Moreover, you might need the function snapToZero after a few calls to avoid
        potential denormalisation issues.
    */
    inline SampleType JUCE_VECTOR_CALLTYPE processSample (SampleType x) noexcept
    {
        return processSample (x, coefficients->getRawCoefficients());
    }

    /** Processes a single sample, sqecialized for
     *  first-order filter.
    */
    template <int N = order>
    inline typename std::enable_if<N == 1, SampleType>::type
        JUCE_VECTOR_CALLTYPE
        processSample (SampleType x, NumericType* c) noexcept
    {
        auto y = state[0] + x * c[0];
        state[0] = x * c[1] - y * c[2];
        return y;
    }

    /** Processes a single sample, sqecialized for
     *  second-order filter.
    */
    template <int N = order>
    inline typename std::enable_if<N == 2, SampleType>::type
        JUCE_VECTOR_CALLTYPE
        processSample (SampleType x, NumericType* c) noexcept
    {
        auto y = x * c[0] + state[0];
        state[0] = x * c[1] - y * c[3] + state[1];
        state[1] = x * c[2] - y * c[4];
        return y;
    }

    /** Processes a single sample, sqecialized for
     *  filter orders greater than 2.
    */
    template <int N = order>
    inline typename std::enable_if<(N > 2), SampleType>::type
        JUCE_VECTOR_CALLTYPE
        processSample (SampleType x, NumericType* c) noexcept
    {
        auto y = (c[0] * x) + state[0];
        for (size_t j = 0; j < order - 1; ++j)
            state[j] = (c[j + 1] * x) - (c[order + j + 1] * y) + state[j + 1];

        state[order - 1] = (c[order] * x) - (c[order * 2] * y);
        return y;
    }

    /** Ensure that the state variables are rounded to zero if the state
        variables are denormals. This is only needed if you are doing
        sample by sample processing.
    */
    void snapToZero() noexcept;

protected:
    SampleType state[order];

private:
    /** Processes a block of samples */
    template <typename ProcessContext, bool isBypassed>
    void processInternal (const ProcessContext& context) noexcept;

    JUCE_LEAK_DETECTOR (Filter)
};

} // namespace chowdsp::IIR

#include "chowdsp_juce_IIRCoefs.h"
#include "chowdsp_juce_IIRFilter_Impl.h"
