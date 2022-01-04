#pragma once

namespace chowdsp
{
/** Second-order lowpass filter. */
template <typename T>
class SecondOrderLPF final : public chowdsp::IIRFilter<2, T>
{
    using NumericType = typename SampleTypeHelpers::ElementType<T>::Type;

public:
    SecondOrderLPF() = default;
    SecondOrderLPF (SecondOrderLPF&&) noexcept = default;
    SecondOrderLPF& operator= (SecondOrderLPF&&) noexcept = default;

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, and sample rate.
     * The analog prototype transfer function is: \f$ H(s) = \frac{1}{s^2 + s/Q + 1} \f$
     */
    void calcCoefs (T fc, T qVal, NumericType fs);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SecondOrderLPF)
};

/** Second-order highpass filter. */
template <typename T>
class SecondOrderHPF final : public chowdsp::IIRFilter<2, T>
{
    using NumericType = typename SampleTypeHelpers::ElementType<T>::Type;

public:
    SecondOrderHPF() = default;
    SecondOrderHPF (SecondOrderHPF&&) noexcept = default;
    SecondOrderHPF& operator= (SecondOrderHPF&&) noexcept = default;

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, and sample rate.
     * The analog prototype transfer function is: \f$ H(s) = \frac{s^2}{s^2 + s/Q + 1} \f$
     */
    void calcCoefs (T fc, T qVal, NumericType fs);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SecondOrderHPF)
};

/** Second-order bandpass filter. */
template <typename T>
class SecondOrderBPF final : public chowdsp::IIRFilter<2, T>
{
    using NumericType = typename SampleTypeHelpers::ElementType<T>::Type;

public:
    SecondOrderBPF() = default;
    SecondOrderBPF (SecondOrderBPF&&) noexcept = default;
    SecondOrderBPF& operator= (SecondOrderBPF&&) noexcept = default;

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, and sample rate.
     * The analog prototype transfer function is: \f$ H(s) = \frac{s/Q}{s^2 + s/Q + 1} \f$
     */
    void calcCoefs (T fc, T qVal, NumericType fs);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SecondOrderBPF)
};

/** Second-order notch filter. */
template <typename T>
class NotchFilter final : public chowdsp::IIRFilter<2, T>
{
    using NumericType = typename SampleTypeHelpers::ElementType<T>::Type;

public:
    NotchFilter() = default;
    NotchFilter (NotchFilter&&) noexcept = default;
    NotchFilter& operator= (NotchFilter&&) noexcept = default;

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, and sample rate.
     * The analog prototype transfer function is: \f$ H(S) = \frac{s^2 + 1}{s^2 + s/Q + 1} \f$
     */
    void calcCoefs (T fc, T qVal, NumericType fs);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NotchFilter)
};

/** Second-order peaking filter. */
template <typename T>
class PeakingFilter final : public chowdsp::IIRFilter<2, T>
{
    using NumericType = typename SampleTypeHelpers::ElementType<T>::Type;

public:
    PeakingFilter() = default;
    PeakingFilter (PeakingFilter&&) noexcept = default;
    PeakingFilter& operator= (PeakingFilter&&) noexcept = default;

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, gain, and sample rate.
     * Note that the gain should be in units of linear gain, NOT Decibels.
     * The analog prototype transfer function is: \f$ H(s) = \frac{s^2 + G s/Q + 1}{s^2 + s/Q + 1} \f$
     */
    void calcCoefs (T fc, T qVal, T gain, NumericType fs);

    /**
     * Calculates the filter coefficients for a given cutoff frequency,
     * Q value, gain (in Decibels), and sample rate.
     */
    void calcCoefsDB (T fc, T qVal, T gainDB, NumericType fs);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PeakingFilter)
};

/** Second-order low-shelf filter. */
template <typename T>
class LowShelfFilter final : public chowdsp::IIRFilter<2, T>
{
    using NumericType = typename SampleTypeHelpers::ElementType<T>::Type;

public:
    LowShelfFilter() = default;
    LowShelfFilter (LowShelfFilter&&) noexcept = default;
    LowShelfFilter& operator= (LowShelfFilter&&) noexcept = default;

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, gain, and sample rate.
     * Note that the gain should be in units of linear gain, NOT Decibels.
     * The analog prototype transfer function is: \f$ H(s) = A \frac{s^2 + \sqrt{A} s/Q + A}{A s^2 + \sqrt{A} s/Q + 1} \f$
     */
    void calcCoefs (T fc, T qVal, T gain, NumericType fs);

    /**
     * Calculates the filter coefficients for a given cutoff frequency,
     * Q value, gain (in Decibels), and sample rate.
     */
    void calcCoefsDB (T fc, T qVal, T gainDB, NumericType fs);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LowShelfFilter)
};

/** Second-order high-shelf filter. */
template <typename T>
class HighShelfFilter final : public chowdsp::IIRFilter<2, T>
{
    using NumericType = typename SampleTypeHelpers::ElementType<T>::Type;

public:
    HighShelfFilter() = default;
    HighShelfFilter (HighShelfFilter&&) noexcept = default;
    HighShelfFilter& operator= (HighShelfFilter&&) noexcept = default;

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, gain, and sample rate.
     * Note that the gain should be in units of linear gain, NOT Decibels.
     * The analog prototype transfer function is: \f$ H(s) = A \frac{As^2 + \sqrt{A} s/Q + 1}{s^2 + \sqrt{A} s/Q + A} \f$
     */
    void calcCoefs (T fc, T qVal, T gain, NumericType fs);

    /**
     * Calculates the filter coefficients for a given cutoff frequency,
     * Q value, gain (in Decibels), and sample rate.
     */
    void calcCoefsDB (T fc, T qVal, T gainDB, NumericType fs);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HighShelfFilter)
};
} // namespace chowdsp

#include "chowdsp_SecondOrderFilters.cpp"
