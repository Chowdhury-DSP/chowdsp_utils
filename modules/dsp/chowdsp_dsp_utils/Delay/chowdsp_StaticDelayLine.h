#pragma once

namespace chowdsp
{
/**
 * Single-channel delay line using statically allocated memory.
 */
template <typename SampleType, typename InterpolationType = DelayLineInterpolationTypes::None, int maxDelaySamples = 1 << 18>
class StaticDelayLine
{
    using NumericType = SampleTypeHelpers::NumericType<SampleType>;

public:
    StaticDelayLine() = default;

    //    void setDelay (NumericType newDelaySamples)
    //    {
    //        static constexpr auto upperLimit = (NumericType) (maxDelaySamples - 1);
    //        jassert (juce::isPositiveAndNotGreaterThan (newDelayInSamples, upperLimit));
    //
    //        delaySamples = juce::jlimit ((NumericType) 0, upperLimit, newDelaySamples);
    //        delayInt = static_cast<int> (std::floor (delaySamples));
    //        delayFrac = delaySamples - (NumericType) delayInt;
    //
    //        interpolator.updateInternalVariables (delayInt, delayFrac);
    //    }
    //
    //    NumericType getDelay() const noexcept
    //    {
    //        return delaySamples;
    //    }

    void reset()
    {
        z = SampleType {};
        writePointer = 0;

        std::fill (std::begin (buffer), std::end (buffer), SampleType {});
    }

    int& getWritePointer() noexcept { return writePointer; }

    //    void processBlock (const float* bufferIn, float* bufferOut, int numSamples) noexcept
    //    {
    //        ScopedValue wp { writePointer };
    //        int rp = wp.get() + delayInt;
    //        ScopedValue state { z };
    //
    //        for (int n = 0; n < numSamples; ++n)
    //        {
    //            pushSample (bufferIn[n], wp.get());
    //            bufferOut[n] = popSample (rp, state.get());
    //
    //            decrementPointer (wp.get());
    //            decrementPointer (rp);
    //        }
    //    }

    inline void pushSample (SampleType x, int wp) noexcept
    {
        jassert (juce::isPositiveAndBelow (wp, maxDelaySamples));
        buffer[wp] = x;
        buffer[wp + maxDelaySamples] = x;
    }

    template <typename IT = InterpolationType>
    inline std::enable_if_t<! std::is_same_v<IT,
                                             DelayLineInterpolationTypes::Thiran>,
                            SampleType>
        popSample (NumericType rp) noexcept
    {
        jassert (juce::isPositiveAndBelow (rp, maxDelaySamples));
        return interpolator.call (buffer,
                                  (int) rp,
                                  rp - (NumericType) (int) rp);
    }

    template <typename IT = InterpolationType>
    inline std::enable_if_t<std::is_same_v<IT,
                                           DelayLineInterpolationTypes::Thiran>,
                            SampleType>
        popSample (NumericType rp, SampleType& state) noexcept
    {
        jassert (juce::isPositiveAndBelow (rp, maxDelaySamples));
        return interpolator.call (buffer,
                                  (int) rp,
                                  rp - (NumericType) (int) rp,
                                  state);
    }

    template <typename T>
    static inline void decrementPointer (T& p) noexcept
    {
        p += T (maxDelaySamples - 1);
        p = p >= (T) maxDelaySamples ? p - (T) maxDelaySamples : p;
    }

private:
    InterpolationType interpolator;

    SampleType buffer[2 * (size_t) maxDelaySamples] {};

    //    NumericType delaySamples = 0.0;
    //    NumericType delayFrac = 0.0;
    //    int delayInt = 0;

    int writePointer = 0;

    SampleType z {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StaticDelayLine)
};
} // namespace chowdsp
