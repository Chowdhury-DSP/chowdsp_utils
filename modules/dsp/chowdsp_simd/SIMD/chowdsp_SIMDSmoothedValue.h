#pragma once

namespace chowdsp::SIMDUtils
{
//==============================================================================
/**
    A utility class for values that need smoothing to avoid audio glitches.
    Adapted from the JUCE implementation to support SIMD types.

    A ValueSmoothingTypes::Linear template parameter selects linear smoothing,
    which increments the SmoothedValue linearly towards its target value.

    @code
    SmoothedValue<float, ValueSmoothingTypes::Linear> yourSmoothedValue;
    @endcode

    A ValueSmoothingTypes::Multiplicative template parameter selects
    multiplicative smoothing increments towards the target value.

    @code
    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> yourSmoothedValue;
    @endcode

    Multiplicative smoothing is useful when you are dealing with
    exponential/logarithmic values like volume in dB or frequency in Hz. For
    example a 12 step ramp from 440.0 Hz (A4) to 880.0 Hz (A5) will increase the
    frequency with an equal temperament tuning across the octave. A 10 step
    smoothing from 1.0 (0 dB) to 3.16228 (10 dB) will increase the value in
    increments of 1 dB.

    Note that when you are using multiplicative smoothing you cannot ever reach a
    target value of zero!
*/
template <typename SmFloatType, typename SmoothingType>
class SIMDSmoothedValue : public juce::SmoothedValueBase<SIMDSmoothedValue<xsimd::batch<SmFloatType>, SmoothingType>>
{
public:
    //==============================================================================
    using VecType = xsimd::batch<SmFloatType>;

    /** Constructor. */
    SIMDSmoothedValue() noexcept
        : SIMDSmoothedValue ((SmFloatType) (std::is_same_v<SmoothingType, juce::ValueSmoothingTypes::Linear> ? 0 : 1))
    {
    }

    /** Constructor. */
    explicit SIMDSmoothedValue (SmFloatType initialValue) noexcept
    {
        // Multiplicative smoothed values cannot ever reach 0!
        jassert (! (std::is_same_v<SmoothingType, juce::ValueSmoothingTypes::Multiplicative> && initialValue == 0));

        // Visual Studio can't handle base class initialisation with CRTP
        this->currentValue = initialValue;
        this->target = this->currentValue;
    }

    //==============================================================================
    /** Reset to a new sample rate and ramp length.
        @param sampleRate           The sample rate
        @param rampLengthInSeconds  The duration of the ramp in seconds
    */
    void reset (double sampleRate, double rampLengthInSeconds) noexcept
    {
        jassert (sampleRate > 0 && rampLengthInSeconds >= 0);
        reset ((int) std::floor (rampLengthInSeconds * sampleRate));
    }

    /** Set a new ramp length directly in samples.
        @param numSteps     The number of samples over which the ramp should be active
    */
    void reset (int numSteps) noexcept
    {
        stepsToTarget = numSteps;
        this->setCurrentAndTargetValue (this->target);
    }

    //==============================================================================
    /** Set the next value to ramp towards.
        @param newValue     The new target value
    */
    void setTargetValue (VecType newValue) noexcept
    {
        if (xsimd::all (newValue == this->target))
            return;

        if (stepsToTarget <= 0)
        {
            this->setCurrentAndTargetValue (newValue);
            return;
        }

        // Multiplicative smoothed values cannot ever reach 0!
        jassert (! (std::is_same_v<SmoothingType, juce::ValueSmoothingTypes::Multiplicative> && xsimd::any (newValue == (SmFloatType) 0)));

        this->target = newValue;
        this->countdown = stepsToTarget;

        setStepSize();
    }

    //==============================================================================
    /** Compute the next value.
        @returns Smoothed value
    */
    VecType getNextValue() noexcept
    {
        if (! this->isSmoothing())
            return this->target;

        --(this->countdown);

        if (this->isSmoothing())
            setNextValue();
        else
            this->currentValue = this->target;

        return this->currentValue;
    }

    //==============================================================================
    /** Skip the next numSamples samples.
        This is identical to calling getNextValue numSamples times. It returns
        the new current value.
        @see getNextValue
    */
    VecType skip (int numSamples) noexcept
    {
        if (numSamples >= this->countdown)
        {
            this->setCurrentAndTargetValue (this->target);
            return this->target;
        }

        skipCurrentValue (numSamples);

        this->countdown -= numSamples;
        return this->currentValue;
    }

private:
#if CHOWDSP_USING_JUCE
    /** Hiding this function since applyGain is ill-defined for SIMD types */
    void applyGain (juce::AudioBuffer<SmFloatType>& /*buffer*/, int /*numSamples*/) noexcept {}
#endif

    //==============================================================================
    template <typename T>
    using LinearVoid = std::enable_if_t<std::is_same_v<T, juce::ValueSmoothingTypes::Linear>, void>;

    template <typename T>
    using MultiplicativeVoid = std::enable_if_t<std::is_same_v<T, juce::ValueSmoothingTypes::Multiplicative>, void>;

    //==============================================================================
    template <typename T = SmoothingType>
    LinearVoid<T> setStepSize() noexcept
    {
        step = (this->target - this->currentValue) / (VecType) (SmFloatType) this->countdown;
    }

    template <typename T = SmoothingType>
    MultiplicativeVoid<T> setStepSize()
    {
        step = xsimd::exp ((xsimd::log (xsimd::abs (this->target)) - xsimd::log (xsimd::abs (this->currentValue))) / (VecType) (SmFloatType) this->countdown);
    }

    //==============================================================================
    template <typename T = SmoothingType>
    LinearVoid<T> setNextValue() noexcept
    {
        this->currentValue += step;
    }

    template <typename T = SmoothingType>
    MultiplicativeVoid<T> setNextValue() noexcept
    {
        this->currentValue *= step;
    }

    //==============================================================================
    template <typename T = SmoothingType>
    LinearVoid<T> skipCurrentValue (int numSamples) noexcept
    {
        this->currentValue += step * (SmFloatType) numSamples;
    }

    template <typename T = SmoothingType>
    MultiplicativeVoid<T> skipCurrentValue (int numSamples)
    {
        this->currentValue *= xsimd::pow (step, (VecType) numSamples);
    }

    //==============================================================================
    VecType step = SmFloatType();
    int stepsToTarget = 0;
};
} // namespace chowdsp::SIMDUtils
