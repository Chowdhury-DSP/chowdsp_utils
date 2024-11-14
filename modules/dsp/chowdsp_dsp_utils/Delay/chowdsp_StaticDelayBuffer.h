#pragma once

namespace chowdsp
{
/**
 * Single-channel delay line using statically allocated memory.
 */
template <typename SampleType, typename InterpolationType = DelayLineInterpolationTypes::None, int maxDelaySamples = 1 << 18, typename StorageType = SampleType>
class StaticDelayBuffer
{
    using NumericType = SampleTypeHelpers::NumericType<SampleType>;

public:
    StaticDelayBuffer() = default;

    void reset()
    {
        std::fill (std::begin (buffer), std::end (buffer), StorageType {});
    }

    template <typename IT = InterpolationType>
    inline std::enable_if_t<! std::is_same_v<IT, DelayLineInterpolationTypes::None>,
                            void>
        pushSample (SampleType x, int wp) noexcept
    {
        jassert (juce::isPositiveAndBelow (wp, maxDelaySamples));
        buffer[wp] = static_cast<StorageType> (x);
        buffer[wp + maxDelaySamples] = static_cast<StorageType> (x);
    }

    template <typename IT = InterpolationType>
    inline std::enable_if_t<std::is_same_v<IT, DelayLineInterpolationTypes::None>,
                            void>
        pushSample (SampleType x, int wp) noexcept
    {
        jassert (juce::isPositiveAndBelow (wp, maxDelaySamples));
        buffer[wp] = static_cast<StorageType> (x);
    }

    template <typename IT = InterpolationType>
    inline std::enable_if_t<! (std::is_same_v<IT, DelayLineInterpolationTypes::Thiran> || std::is_same_v<IT, DelayLineInterpolationTypes::None>),
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
                                           DelayLineInterpolationTypes::None>,
                            SampleType>
        popSample (NumericType rp) noexcept
    {
        jassert (juce::isPositiveAndBelow (rp, maxDelaySamples));
        return interpolator.call (buffer, (int) rp);
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

    static inline NumericType getReadPointer (int wp, NumericType delaySamples) noexcept
    {
        delaySamples = juce::jmax ((NumericType) 1, delaySamples);
        const auto rp = std::fmod (NumericType (wp) + delaySamples, (NumericType) maxDelaySamples);
        jassert (juce::isPositiveAndBelow (rp, (NumericType) maxDelaySamples));
        return rp;
    }

    static inline int getReadPointer (int wp, int delaySamples) noexcept
    {
        delaySamples = juce::jmax (1, delaySamples);
        const auto rp = (wp + delaySamples) % maxDelaySamples;
        jassert (juce::isPositiveAndBelow (rp, maxDelaySamples));
        return rp;
    }

private:
    InterpolationType interpolator;

    static constexpr auto bufferSize = size_t (std::is_same_v<InterpolationType, DelayLineInterpolationTypes::None> ? maxDelaySamples : (2 * maxDelaySamples));
    StorageType buffer[bufferSize] {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StaticDelayBuffer)
};
} // namespace chowdsp
