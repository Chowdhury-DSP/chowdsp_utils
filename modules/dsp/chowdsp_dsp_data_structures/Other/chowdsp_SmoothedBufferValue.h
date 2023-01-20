#pragma once

#if JUCE_MODULE_AVAILABLE_chowdsp_parameters
#include <chowdsp_parameters/chowdsp_parameters.h>
#endif

#if ! CHOWDSP_USING_JUCE
namespace juce
{
}
#endif

namespace chowdsp
{
#ifndef DOXYGEN
class FloatParameter;
#endif

/**
 * Template class for smoothing a value over a series of buffers.
 * This can be used with raw values or with parameter handles.
 */
template <typename FloatType, typename ValueSmoothingType = juce::ValueSmoothingTypes::Linear>
class SmoothedBufferValue
{
public:
    using NumericType = FloatType;
    using SmoothingType = ValueSmoothingType;

    /** Default constructor */
    SmoothedBufferValue() = default;

    /**
     * Sets a parameter handle for this buffer to use for smoothing.
     * Note that the parameter handle must not be deleted before this object!
     *
     * @param handle A parameter handle to use for smoothing
     */
    void setParameterHandle (std::atomic<float>* handle);

    /**
     * Sets a parameter handle for this buffer to use for smoothing.
     * Note that the parameter handle must not be deleted before this object!
     *
     * @param handle A parameter handle to use for smoothing
     */
    void setParameterHandle (FloatParameter* handle);

    /** Prepare the smoother to process samples with a given sample rate and block size. */
    void prepare (double sampleRate, int samplesPerBlock);

    /** Resets the state of the smoother with a given value. */
    void reset (FloatType resetValue);

    /** Resets the state of the smoother. */
    void reset();

    /** Sets the ramp length to use for smoothing. */
    void setRampLength (double rampLengthSeconds);

    /**
     * Returns true if the value has been smoothed over the most recently
     * processed buffer.
     */
    [[nodiscard]] bool isSmoothing() const noexcept { return isCurrentlySmoothing; }

    /** Returns the current smoothed value */
    [[nodiscard]] FloatType getCurrentValue() const noexcept { return smoother.getCurrentValue(); }

    /**
     * Process smoothing for the current parameter handle.
     * Please don't call this function if the parameter handle has nt been set!
     */
    void process (int numSamples);

    /**
     * Process smoothing for the input value.
     * If smoothing an audio parameter, it is recommended to use a parameter handle instead!
     */
    void process (FloatType value, int numSamples);

    /** Returns a pointer to the current smoothed buffer. */
    [[nodiscard]] const FloatType* getSmoothedBuffer() const { return buffer.data(); }

    /**
     * Optional mapping function to map from the set value to the smoothed value.
     *
     * If using a custom mapping function, make sure this is set properly before calling
     * `prepare()` or `reset()`.
     */
    std::function<FloatType (FloatType)> mappingFunction = [] (auto x)
    { return x; };

private:
#if ! CHOWDSP_NO_XSIMD
    std::vector<FloatType, xsimd::default_allocator<FloatType>> buffer;
#else
    std::vector<FloatType> buffer;
#endif
    juce::SmoothedValue<FloatType, ValueSmoothingType> smoother;
    bool isCurrentlySmoothing = false;

    std::atomic<float>* parameterHandle = nullptr;

    FloatParameter* modulatableParameterHandle = nullptr;

    double sampleRate = 48000.0;
    double rampLengthInSeconds = 0.05;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SmoothedBufferValue)
};
} // namespace chowdsp
