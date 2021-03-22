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
    /** A set of coefficients for use in an Filter object.
    @see IIR::Filter
*/
    template <typename NumericType, size_t order>
    struct Coefficients : public juce::dsp::ProcessorState
    {
        /** Creates a null set of coefficients (which will produce silence). */
        Coefficients()
        {
            std::fill (coefficients.begin(), coefficients.end(), NumericType());
        }

        /** Directly constructs an object from the raw coefficients.
        Most people will want to use the static methods instead of this, but the
        constructor is public to allow tinkerers to create their own custom filters!
    */
        // template <class = typename std::enable_if<IsFirstOrder>::type>
        template <size_t N = order, typename = typename std::enable_if<N == 1>::type>
        Coefficients (NumericType b0, NumericType b1, NumericType a0, NumericType a1)
        {
            jassert (a0 != 0);

            auto a0inv = static_cast<NumericType> (1) / a0;

            coefficients = { b0 * a0inv,
                             b1 * a0inv,
                             a1 * a0inv };
        }

        template <size_t N = order, typename = typename std::enable_if<N == 2>::type>
        Coefficients (NumericType b0, NumericType b1, NumericType b2, NumericType a0, NumericType a1, NumericType a2)
        {
            jassert (a0 != 0);

            auto a0inv = static_cast<NumericType> (1) / a0;

            coefficients = { b0 * a0inv,
                             b1 * a0inv,
                             b2 * a0inv,
                             a1 * a0inv,
                             a2 * a0inv };
        }

        template <size_t N = order, typename = typename std::enable_if<N == 3>::type>
        Coefficients (NumericType b0, NumericType b1, NumericType b2, NumericType b3, NumericType a0, NumericType a1, NumericType a2, NumericType a3)
        {
            jassert (a0 != 0);

            auto a0inv = static_cast<NumericType> (1) / a0;

            coefficients = { b0 * a0inv,
                             b1 * a0inv,
                             b2 * a0inv,
                             b3 * a0inv,
                             a1 * a0inv,
                             a2 * a0inv,
                             a3 * a0inv };
        }

        Coefficients (const Coefficients&) = default;
        Coefficients (Coefficients&&) = default;
        Coefficients& operator= (const Coefficients&) = default;
        Coefficients& operator= (Coefficients&&) = default;

        /** The Coefficients structure is ref-counted, so this is a handy type that can be used
        as a pointer to one.
    */
        using Ptr = juce::ReferenceCountedObjectPtr<Coefficients>;

        //==============================================================================
        /** Returns the coefficients for a first order low-pass filter. */
        static juce::ReferenceCountedObjectPtr<Coefficients<NumericType, 1>>
            makeFirstOrderLowPass (double sampleRate, NumericType frequency)
        {
            jassert (sampleRate > 0.0);
            jassert (frequency > 0 && frequency <= static_cast<float> (sampleRate * 0.5));

            auto n = std::tan (juce::MathConstants<NumericType>::pi * frequency / static_cast<NumericType> (sampleRate));

            return *new Coefficients (n, n, n + 1, n - 1);
        }

        /** Returns the coefficients for a first order high-pass filter. */
        static juce::ReferenceCountedObjectPtr<Coefficients<NumericType, 1>>
            makeFirstOrderHighPass (double sampleRate, NumericType frequency)
        {
            jassert (sampleRate > 0.0);
            jassert (frequency > 0 && frequency <= static_cast<float> (sampleRate * 0.5));

            auto n = std::tan (juce::MathConstants<NumericType>::pi * frequency / static_cast<NumericType> (sampleRate));

            return *new Coefficients (1, -1, n + 1, n - 1);
        }

        /** Returns the coefficients for a first order all-pass filter. */
        static juce::ReferenceCountedObjectPtr<Coefficients<NumericType, 1>>
            makeFirstOrderAllPass (double sampleRate, NumericType frequency)
        {
            jassert (sampleRate > 0.0);
            jassert (frequency > 0 && frequency <= static_cast<float> (sampleRate * 0.5));

            auto n = std::tan (juce::MathConstants<NumericType>::pi * frequency / static_cast<NumericType> (sampleRate));

            return *new Coefficients (n - 1, n + 1, n + 1, n - 1);
        }

        //==============================================================================
        /** Returns the coefficients for a low-pass filter. */
        static juce::ReferenceCountedObjectPtr<Coefficients<NumericType, 2>>
            makeLowPass (double sampleRate, NumericType frequency)
        {
            return makeLowPass (sampleRate, frequency, inverseRootTwo);
        }

        /** Returns the coefficients for a low-pass filter with variable Q. */
        static juce::ReferenceCountedObjectPtr<Coefficients<NumericType, 2>>
            makeLowPass (double sampleRate, NumericType frequency, NumericType Q)
        {
            jassert (sampleRate > 0.0);
            jassert (frequency > 0 && frequency <= static_cast<float> (sampleRate * 0.5));
            jassert (Q > 0.0);

            auto n = 1 / std::tan (juce::MathConstants<NumericType>::pi * frequency / static_cast<NumericType> (sampleRate));
            auto nSquared = n * n;
            auto invQ = 1 / Q;
            auto c1 = 1 / (1 + invQ * n + nSquared);

            return *new Coefficients (c1, c1 * 2, c1, 1, c1 * 2 * (1 - nSquared), c1 * (1 - invQ * n + nSquared));
        }

        //==============================================================================
        /** Returns the coefficients for a high-pass filter. */
        static juce::ReferenceCountedObjectPtr<Coefficients<NumericType, 2>>
            makeHighPass (double sampleRate, NumericType frequency)
        {
            return makeHighPass (sampleRate, frequency, inverseRootTwo);
        }

        /** Returns the coefficients for a high-pass filter with variable Q. */
        static juce::ReferenceCountedObjectPtr<Coefficients<NumericType, 2>>
            makeHighPass (double sampleRate, NumericType frequency, NumericType Q)
        {
            jassert (sampleRate > 0.0);
            jassert (frequency > 0 && frequency <= static_cast<float> (sampleRate * 0.5));
            jassert (Q > 0.0);

            auto n = std::tan (juce::MathConstants<NumericType>::pi * frequency / static_cast<NumericType> (sampleRate));
            auto nSquared = n * n;
            auto invQ = 1 / Q;
            auto c1 = 1 / (1 + invQ * n + nSquared);

            return *new Coefficients (c1, c1 * -2, c1, 1, c1 * 2 * (nSquared - 1), c1 * (1 - invQ * n + nSquared));
        }

        //==============================================================================
        /** Returns the coefficients for a band-pass filter. */
        static juce::ReferenceCountedObjectPtr<Coefficients<NumericType, 2>>
            makeBandPass (double sampleRate, NumericType frequency)
        {
            return makeBandPass (sampleRate, frequency, inverseRootTwo);
        }

        /** Returns the coefficients for a band-pass filter with variable Q. */
        static juce::ReferenceCountedObjectPtr<Coefficients<NumericType, 2>>
            makeBandPass (double sampleRate, NumericType frequency, NumericType Q)
        {
            jassert (sampleRate > 0.0);
            jassert (frequency > 0 && frequency <= static_cast<float> (sampleRate * 0.5));
            jassert (Q > 0.0);

            auto n = 1 / std::tan (juce::MathConstants<NumericType>::pi * frequency / static_cast<NumericType> (sampleRate));
            auto nSquared = n * n;
            auto invQ = 1 / Q;
            auto c1 = 1 / (1 + invQ * n + nSquared);

            return *new Coefficients (c1 * n * invQ, 0, -c1 * n * invQ, 1, c1 * 2 * (1 - nSquared), c1 * (1 - invQ * n + nSquared));
        }

        //==============================================================================
        /** Returns the coefficients for a notch filter. */
        static juce::ReferenceCountedObjectPtr<Coefficients<NumericType, 2>>
            makeNotch (double sampleRate, NumericType frequency)
        {
            return makeNotch (sampleRate, frequency, inverseRootTwo);
        }

        /** Returns the coefficients for a notch filter with variable Q. */
        static juce::ReferenceCountedObjectPtr<Coefficients<NumericType, 2>>
            makeNotch (double sampleRate, NumericType frequency, NumericType Q)
        {
            jassert (sampleRate > 0.0);
            jassert (frequency > 0 && frequency <= static_cast<float> (sampleRate * 0.5));
            jassert (Q > 0.0);

            auto n = 1 / std::tan (juce::MathConstants<NumericType>::pi * frequency / static_cast<NumericType> (sampleRate));
            auto nSquared = n * n;
            auto invQ = 1 / Q;
            auto c1 = 1 / (1 + n * invQ + nSquared);
            auto b0 = c1 * (1 + nSquared);
            auto b1 = 2 * c1 * (1 - nSquared);

            return *new Coefficients (b0, b1, b0, 1, b1, c1 * (1 - n * invQ + nSquared));
        }

        //==============================================================================
        /** Returns the coefficients for an all-pass filter. */
        static juce::ReferenceCountedObjectPtr<Coefficients<NumericType, 2>>
            makeAllPass (double sampleRate, NumericType frequency)
        {
            return makeAllPass (sampleRate, frequency, inverseRootTwo);
        }

        /** Returns the coefficients for an all-pass filter with variable Q. */
        static juce::ReferenceCountedObjectPtr<Coefficients<NumericType, 2>>
            makeAllPass (double sampleRate, NumericType frequency, NumericType Q)
        {
            jassert (sampleRate > 0);
            jassert (frequency > 0 && frequency <= sampleRate * 0.5);
            jassert (Q > 0);

            auto n = 1 / std::tan (juce::MathConstants<NumericType>::pi * frequency / static_cast<NumericType> (sampleRate));
            auto nSquared = n * n;
            auto invQ = 1 / Q;
            auto c1 = 1 / (1 + invQ * n + nSquared);
            auto b0 = c1 * (1 - n * invQ + nSquared);
            auto b1 = c1 * 2 * (1 - nSquared);

            return *new Coefficients (b0, b1, 1, 1, b1, b0);
        }

        //==============================================================================
        /** Returns the coefficients for a low-pass shelf filter with variable Q and gain.

        The gain is a scale factor that the low frequencies are multiplied by, so values
        greater than 1.0 will boost the low frequencies, values less than 1.0 will
        attenuate them.
    */
        static juce::ReferenceCountedObjectPtr<Coefficients<NumericType, 2>>
            makeLowShelf (double sampleRate, NumericType cutOffFrequency, NumericType Q, NumericType gainFactor)
        {
            jassert (sampleRate > 0.0);
            jassert (cutOffFrequency > 0.0 && cutOffFrequency <= sampleRate * 0.5);
            jassert (Q > 0.0);

            auto A = juce::jmax (static_cast<NumericType> (0.0), std::sqrt (gainFactor));
            auto aminus1 = A - 1;
            auto aplus1 = A + 1;
            auto omega = (2 * juce::MathConstants<NumericType>::pi * juce::jmax (cutOffFrequency, static_cast<NumericType> (2.0))) / static_cast<NumericType> (sampleRate);
            auto coso = std::cos (omega);
            auto beta = std::sin (omega) * std::sqrt (A) / Q;
            auto aminus1TimesCoso = aminus1 * coso;

            return *new Coefficients (A * (aplus1 - aminus1TimesCoso + beta),
                                      A * 2 * (aminus1 - aplus1 * coso),
                                      A * (aplus1 - aminus1TimesCoso - beta),
                                      aplus1 + aminus1TimesCoso + beta,
                                      -2 * (aminus1 + aplus1 * coso),
                                      aplus1 + aminus1TimesCoso - beta);
        }

        /** Returns the coefficients for a high-pass shelf filter with variable Q and gain.

        The gain is a scale factor that the high frequencies are multiplied by, so values
        greater than 1.0 will boost the high frequencies, values less than 1.0 will
        attenuate them.
    */
        static juce::ReferenceCountedObjectPtr<Coefficients<NumericType, 2>>
            makeHighShelf (double sampleRate, NumericType cutOffFrequency, NumericType Q, NumericType gainFactor)
        {
            jassert (sampleRate > 0);
            jassert (cutOffFrequency > 0 && cutOffFrequency <= static_cast<NumericType> (sampleRate * 0.5));
            jassert (Q > 0);

            auto A = juce::jmax (static_cast<NumericType> (0.0), std::sqrt (gainFactor));
            auto aminus1 = A - 1;
            auto aplus1 = A + 1;
            auto omega = (2 * juce::MathConstants<NumericType>::pi * juce::jmax (cutOffFrequency, static_cast<NumericType> (2.0))) / static_cast<NumericType> (sampleRate);
            auto coso = std::cos (omega);
            auto beta = std::sin (omega) * std::sqrt (A) / Q;
            auto aminus1TimesCoso = aminus1 * coso;

            return *new Coefficients (A * (aplus1 + aminus1TimesCoso + beta),
                                      A * -2 * (aminus1 + aplus1 * coso),
                                      A * (aplus1 + aminus1TimesCoso - beta),
                                      aplus1 - aminus1TimesCoso + beta,
                                      2 * (aminus1 - aplus1 * coso),
                                      aplus1 - aminus1TimesCoso - beta);
        }

        /** Returns the coefficients for a peak filter centred around a
        given frequency, with a variable Q and gain.

        The gain is a scale factor that the centre frequencies are multiplied by, so
        values greater than 1.0 will boost the centre frequencies, values less than
        1.0 will attenuate them.
    */
        static juce::ReferenceCountedObjectPtr<Coefficients<NumericType, 2>>
            makePeakFilter (double sampleRate, NumericType frequency, NumericType Q, NumericType gainFactor)
        {
            jassert (sampleRate > 0);
            jassert (frequency > 0 && frequency <= static_cast<NumericType> (sampleRate * 0.5));
            jassert (Q > 0);
            jassert (gainFactor > 0);

            auto A = juce::jmax (static_cast<NumericType> (0.0), std::sqrt (gainFactor));
            auto omega = (2 * juce::MathConstants<NumericType>::pi * juce::jmax (frequency, static_cast<NumericType> (2.0))) / static_cast<NumericType> (sampleRate);
            auto alpha = std::sin (omega) / (Q * 2);
            auto c2 = -2 * std::cos (omega);
            auto alphaTimesA = alpha * A;
            auto alphaOverA = alpha / A;

            return *new Coefficients (1 + alphaTimesA, c2, 1 - alphaTimesA, 1 + alphaOverA, c2, 1 - alphaOverA);
        }

        //==============================================================================
        /** Returns the filter order associated with the coefficients */
        constexpr size_t getFilterOrder() const noexcept { return order; }

        /** Returns the magnitude frequency response of the filter for a given frequency
        and sample rate
    */
        double getMagnitudeForFrequency (double frequency, double sampleRate) const noexcept
        {
            constexpr juce::dsp::Complex<double> j (0, 1);
            const auto* coefs = coefficients.begin();

            jassert (frequency >= 0 && frequency <= sampleRate * 0.5);

            juce::dsp::Complex<double> numerator = 0.0, denominator = 0.0, factor = 1.0;
            juce::dsp::Complex<double> jw = std::exp (-juce::MathConstants<double>::twoPi * frequency * j / sampleRate);

            for (size_t n = 0; n <= order; ++n)
            {
                numerator += static_cast<double> (coefs[n]) * factor;
                factor *= jw;
            }

            denominator = 1.0;
            factor = jw;

            for (size_t n = order + 1; n <= 2 * order; ++n)
            {
                denominator += static_cast<double> (coefs[n]) * factor;
                factor *= jw;
            }

            return std::abs (numerator / denominator);
        }

        /** Returns the magnitude frequency response of the filter for a given frequency array
        and sample rate.
    */
        void getMagnitudeForFrequencyArray (const double* frequencies, double* magnitudes, size_t numSamples, double sampleRate) const noexcept
        {
            constexpr juce::dsp::Complex<double> j (0, 1);
            const auto* coefs = coefficients.begin();

            jassert (order >= 0);

            for (size_t i = 0; i < numSamples; ++i)
            {
                jassert (frequencies[i] >= 0 && frequencies[i] <= sampleRate * 0.5);

                juce::dsp::Complex<double> numerator = 0.0, denominator = 0.0, factor = 1.0;
                juce::dsp::Complex<double> jw = std::exp (-juce::MathConstants<double>::twoPi * frequencies[i] * j / sampleRate);

                for (size_t n = 0; n <= order; ++n)
                {
                    numerator += static_cast<double> (coefs[n]) * factor;
                    factor *= jw;
                }

                denominator = 1.0;
                factor = jw;

                for (size_t n = order + 1; n <= 2 * order; ++n)
                {
                    denominator += static_cast<double> (coefs[n]) * factor;
                    factor *= jw;
                }

                magnitudes[i] = std::abs (numerator / denominator);
            }
        }

        /** Returns the phase frequency response of the filter for a given frequency and
        sample rate
    */
        double getPhaseForFrequency (double frequency, double sampleRate) const noexcept
        {
            constexpr juce::dsp::Complex<double> j (0, 1);
            const auto* coefs = coefficients.begin();

            jassert (frequency >= 0 && frequency <= sampleRate * 0.5);

            juce::dsp::Complex<double> numerator = 0.0, denominator = 0.0, factor = 1.0;
            juce::dsp::Complex<double> jw = std::exp (-juce::MathConstants<double>::twoPi * frequency * j / sampleRate);

            for (size_t n = 0; n <= order; ++n)
            {
                numerator += static_cast<double> (coefs[n]) * factor;
                factor *= jw;
            }

            denominator = 1.0;
            factor = jw;

            for (size_t n = order + 1; n <= 2 * order; ++n)
            {
                denominator += static_cast<double> (coefs[n]) * factor;
                factor *= jw;
            }

            return std::arg (numerator / denominator);
        }

        /** Returns the phase frequency response of the filter for a given frequency array
        and sample rate.
    */
        void getPhaseForFrequencyArray (double* frequencies, double* phases, size_t numSamples, double sampleRate) const noexcept
        {
            jassert (sampleRate > 0);

            constexpr juce::dsp::Complex<double> j (0, 1);
            const auto* coefs = coefficients.begin();
            auto invSampleRate = 1 / sampleRate;

            jassert (order >= 0);

            for (size_t i = 0; i < numSamples; ++i)
            {
                jassert (frequencies[i] >= 0 && frequencies[i] <= sampleRate * 0.5);

                juce::dsp::Complex<double> numerator = 0.0, denominator = 0.0, factor = 1.0;
                juce::dsp::Complex<double> jw = std::exp (-juce::MathConstants<double>::twoPi * frequencies[i] * j * invSampleRate);

                for (size_t n = 0; n <= order; ++n)
                {
                    numerator += static_cast<double> (coefs[n]) * factor;
                    factor *= jw;
                }

                denominator = 1.0;
                factor = jw;

                for (size_t n = order + 1; n <= 2 * order; ++n)
                {
                    denominator += static_cast<double> (coefs[n]) * factor;
                    factor *= jw;
                }

                phases[i] = std::arg (numerator / denominator);
            }
        }

        /** Returns a raw data pointer to the coefficients. */
        NumericType* getRawCoefficients() noexcept { return coefficients.data(); }

        /** Returns a raw data pointer to the coefficients. */
        const NumericType* getRawCoefficients() const noexcept { return coefficients.begin(); }

        //==============================================================================
        /** The raw coefficients.
        You should leave these numbers alone unless you really know what you're doing.
    */
        std::array<NumericType, 2 * order + 1> coefficients;

    private:
        // Unfortunately, std::sqrt is not marked as constexpr just yet in all compilers
        static constexpr NumericType inverseRootTwo = static_cast<NumericType> (0.70710678118654752440L);
    };

} // namespace IIR
} // namespace chowdsp
