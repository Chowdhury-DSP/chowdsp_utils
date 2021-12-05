#pragma once

namespace chowdsp
{
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
     *          s^2 + (1 / Q) * G * s + 1
     *  H(s) = ---------------------------
     *           s^2 + (1 / Q) * s  +  1
     */
    void calcCoefs (T fc, T qVal, T gain, T fs)
    {
        using namespace Bilinear;

        const auto wc = juce::MathConstants<T>::twoPi * fc;
        const auto K = computeKValue (fc, fs);

        auto kSqTerm = (T) 1 / (wc * wc);
        auto kTerm = (T) 1 / (qVal * wc);
        auto kNum = gain > (T) 1 ? kTerm * gain : kTerm;
        auto kDen = gain < (T) 1 ? kTerm / gain : kTerm;

        BilinearTransform<T, 3>::call (this->b, this->a, { kSqTerm, kNum, (T) 1 }, { kSqTerm, kDen, (T) 1 }, K);
    }

    /**
     * Calculates the filter coefficients for a given cutoff frequency,
     * Q value, gain (in Decibels), and sample rate.
     */
    void calcCoefsDB (T fc, T qVal, T gainDB, T fs)
    {
        calcCoefs (fc, qVal, juce::Decibels::decibelsToGain (gainDB), fs);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PeakingFilter)
};
} // namespace chowdsp
