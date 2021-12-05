#pragma once

namespace chowdsp
{
/** Second-order lowpass filter. */
template <typename T>
class SecondOrderLPF final : public chowdsp::IIRFilter<2, T>
{
public:
    SecondOrderLPF() = default;

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, and sample rate.
     * The analog prototype transfer function is:
     *                    1
     *  H(s) = -----------------------
     *           s^2 + (s / Q) +  1
     */
    void calcCoefs (T fc, T qVal, T fs);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SecondOrderLPF)
};

/** Second-order highpass filter. */
template <typename T>
class SecondOrderHPF final : public chowdsp::IIRFilter<2, T>
{
public:
    SecondOrderHPF() = default;

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, and sample rate.
     * The analog prototype transfer function is:
     *                   s^2
     *  H(s) = ------------------------
     *           s^2 + (s / Q)  +  1
     */
    void calcCoefs (T fc, T qVal, T fs);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SecondOrderHPF)
};

/** Second-order bandpass filter. */
template <typename T>
class SecondOrderBPF final : public chowdsp::IIRFilter<2, T>
{
public:
    SecondOrderBPF() = default;

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, and sample rate.
     * The analog prototype transfer function is:
     *                 s / Q
     *  H(s) = ---------------------
     *          s^2 + (s / Q) +  1
     */
    void calcCoefs (T fc, T qVal, T fs);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SecondOrderBPF)
};

/** Second-order notch filter. */
template <typename T>
class NotchFilter final : public chowdsp::IIRFilter<2, T>
{
public:
    NotchFilter() = default;

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, and sample rate.
     * The analog prototype transfer function is:
     *                s^2 + 1
     *  H(s) = ---------------------
     *          s^2 + (s / Q) +  1
     */
    void calcCoefs (T fc, T qVal, T fs);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NotchFilter)
};

/** Second-order peaking filter. */
template <typename T>
class PeakingFilter final : public chowdsp::IIRFilter<2, T>
{
public:
    PeakingFilter() = default;

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, gain, and sample rate.
     * Note that the gain should be in units of linear gain, NOT Decibels.
     * The analog prototype transfer function is:
     *          s^2 + G * (s / Q) + 1
     *  H(s) = -----------------------
     *            s^2 + (s / Q) + 1
     */
    void calcCoefs (T fc, T qVal, T gain, T fs);

    /**
     * Calculates the filter coefficients for a given cutoff frequency,
     * Q value, gain (in Decibels), and sample rate.
     */
    void calcCoefsDB (T fc, T qVal, T gainDB, T fs);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PeakingFilter)
};

/** Second-order low-shelf filter. */
template <typename T>
class LowShelfFilter final : public chowdsp::IIRFilter<2, T>
{
public:
    LowShelfFilter() = default;

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, gain, and sample rate.
     * Note that the gain should be in units of linear gain, NOT Decibels.
     * The analog prototype transfer function is:
     *              s^2 + sqrt(A) * (s / Q) + A
     *  H(s) = A ---------------------------------
     *            A * s^2 + sqrt(A) * (s / Q) + 1
     */
    void calcCoefs (T fc, T qVal, T gain, T fs);

    /**
     * Calculates the filter coefficients for a given cutoff frequency,
     * Q value, gain (in Decibels), and sample rate.
     */
    void calcCoefsDB (T fc, T qVal, T gainDB, T fs);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LowShelfFilter)
};

/** Second-order high-shelf filter. */
template <typename T>
class HighShelfFilter final : public chowdsp::IIRFilter<2, T>
{
public:
    HighShelfFilter() = default;

    /**
     * Calculates the filter coefficients for a given cutoff frequency, Q value, gain, and sample rate.
     * Note that the gain should be in units of linear gain, NOT Decibels.
     * The analog prototype transfer function is:
     *            A * s^2 + sqrt(A) * (s / Q) + 1
     *  H(s) = A ---------------------------------
     *              s^2 + sqrt(A) * (s / Q) + A
     */
    void calcCoefs (T fc, T qVal, T gain, T fs);

    /**
     * Calculates the filter coefficients for a given cutoff frequency,
     * Q value, gain (in Decibels), and sample rate.
     */
    void calcCoefsDB (T fc, T qVal, T gainDB, T fs);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HighShelfFilter)
};
} // namespace chowdsp

#include "chowdsp_SecondOrderFilters.cpp"
