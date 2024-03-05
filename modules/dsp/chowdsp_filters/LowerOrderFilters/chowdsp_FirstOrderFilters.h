#pragma once

#include "../Utils/chowdsp_CoefficientCalculators.h"

namespace chowdsp
{
/**
 * First-order lowpass filter (-6 dB / octave).
 * This filter can be used to model an ideal RC lowpwass filter.
 */
template <typename T, size_t maxChannelCount = defaultChannelCount>
class FirstOrderLPF final : public IIRFilter<1, T, maxChannelCount>
{
public:
    using NumericType = SampleTypeHelpers::ProcessorNumericType<FirstOrderLPF>;

    FirstOrderLPF() = default;
    FirstOrderLPF (FirstOrderLPF&&) noexcept = default;
    FirstOrderLPF& operator= (FirstOrderLPF&&) noexcept = default;

    /**
     * Calculates the filter coefficients for a given cutoff frequency and sample rate.
     * The analog prototype transfer function is: \f$ H(s) = \frac{1}{s + 1} \f$
     */
    void calcCoefs (T fc, NumericType fs)
    {
        CoefficientCalculators::calcFirstOrderLPF (this->b, this->a, fc, fs);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FirstOrderLPF)
};

/**
 * First-order highpass filter (-6 dB / octave).
 * This filter can be used to model an ideal RC highpass filter.
 */
template <typename T, size_t maxChannelCount = defaultChannelCount>
class FirstOrderHPF final : public IIRFilter<1, T, maxChannelCount>
{
public:
    using NumericType = SampleTypeHelpers::ProcessorNumericType<FirstOrderHPF>;

    FirstOrderHPF() = default;
    FirstOrderHPF (FirstOrderHPF&&) noexcept = default;
    FirstOrderHPF& operator= (FirstOrderHPF&&) noexcept = default;

    /**
     * Calculates the filter coefficients for a given cutoff frequency and sample rate.
     * The analog prototype transfer function is: \f$ H(s) = \frac{s}{s + 1} \f$
     */
    void calcCoefs (T fc, NumericType fs)
    {
        CoefficientCalculators::calcFirstOrderHPF (this->b, this->a, fc, fs);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FirstOrderHPF)
};

/**
 * A first order shelving filter, with a set gain at DC,
 * a set gain at high frequencies, and a transition frequency.
 */
template <typename T = float, size_t maxChannelCount = defaultChannelCount>
class ShelfFilter final : public IIRFilter<1, T, maxChannelCount>
{
public:
    using NumericType = SampleTypeHelpers::ProcessorNumericType<ShelfFilter>;

    ShelfFilter() = default;
    ShelfFilter (ShelfFilter&&) noexcept = default;
    ShelfFilter& operator= (ShelfFilter&&) noexcept = default;

    /** Calculates the coefficients for the filter.
     * @param lowGain: the gain of the filter at low frequencies
     * @param highGain: the gain of the filter at high frequencies
     * @param fc: the transition frequency of the filter
     * @param fs: the sample rate for the filter
     *
     * For information on the filter coefficient derivation,
     * see Abel and Berners dsp4dae, pg. 249
     */
    void calcCoefs (T lowGain, T highGain, T fc, NumericType fs)
    {
        CoefficientCalculators::calcFirstOrderShelf (this->b, this->a, lowGain, highGain, fc, fs);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShelfFilter)
};
} // namespace chowdsp
