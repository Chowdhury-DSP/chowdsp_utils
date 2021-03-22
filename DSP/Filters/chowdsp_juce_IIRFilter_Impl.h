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

namespace chowdsp
{
namespace IIR
{
    //==============================================================================
    template <typename SampleType, size_t order>
    Filter<SampleType, order>::Filter()
        : coefficients (new Coefficients<typename Filter<SampleType>::NumericType, order>)
    {
        reset();
    }

    template <typename SampleType, size_t order>
    Filter<SampleType, order>::Filter (CoefficientsPtr c) : coefficients (std::move (c))
    {
        reset();
    }

    template <typename SampleType, size_t order>
    void Filter<SampleType, order>::reset (SampleType resetToValue)
    {
        std::fill (state, &state[order], resetToValue);
    }

    template <typename SampleType, size_t order>
    void Filter<SampleType, order>::prepare (const juce::dsp::ProcessSpec&) noexcept
    {
        reset();
    }

    template <typename SampleType, size_t order>
    template <typename ProcessContext, bool bypassed>
    void Filter<SampleType, order>::processInternal (const ProcessContext& context) noexcept
    {
        static_assert (std::is_same<typename ProcessContext::SampleType, SampleType>::value,
                       "The sample-type of the IIR filter must match the sample-type supplied to this process callback");

        auto&& inputBlock = context.getInputBlock();
        auto&& outputBlock = context.getOutputBlock();

        // This class can only process mono signals. Use the ProcessorDuplicator class
        // to apply this filter on a multi-channel audio stream.
        jassert (inputBlock.getNumChannels() == 1);
        jassert (outputBlock.getNumChannels() == 1);

        auto numSamples = inputBlock.getNumSamples();
        auto* src = inputBlock.getChannelPointer (0);
        auto* dst = outputBlock.getChannelPointer (0);
        auto* coeffs = coefficients->getRawCoefficients();

        for (size_t i = 0; i < numSamples; ++i)
        {
            auto input = src[i];
            auto output = processSample (input, coeffs);
            dst[i] = bypassed ? input : output;
        }

        snapToZero();
    }

    template <typename SampleType, size_t order>
    void Filter<SampleType, order>::snapToZero() noexcept
    {
        for (size_t i = 0; i < order; ++i)
            juce::dsp::util::snapToZero (state[i]);
    }

} // namespace IIR
} // namespace chowdsp
