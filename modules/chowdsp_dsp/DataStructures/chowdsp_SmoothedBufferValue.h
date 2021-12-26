#pragma once

namespace chowdsp
{
/**
 * Template class for smoothing a value over a series of buffers.
 * This can be used with raw values or with parameter handles.
 */
template <typename FloatType, typename ValueSmoothingType = juce::ValueSmoothingTypes::Linear>
class SmoothedBufferValue
{
public:
    /** Default constructor */
    SmoothedBufferValue() = default;

    /**
     * Sets a parameter handle for this buffer to use for smoothing.
     * Note that the parameter handle must not be deleted before this object!
     *
     * @param handle A parameter handle to use for smoothing
     */
    void setParameterHandle (std::atomic<float>* handle);

    /** Prepare the smoother to process samples with a given sample rate and block size. */
    void prepare (double sampleRate, int samplesPerBlock);

    /** Resets the state of the smoother with a given value. */
    void reset (FloatType resetValue);

    /** Resets the state of the smoother. */
    void reset();

    /** Sets the ramp length to use for smoothing. */
    void setRampLength (double rampLengthSeconds);

    /** Returns true if the value is currently being smoothed */
    bool isSmoothing() const noexcept { return smoother.isSmoothing(); }

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
    const FloatType* getSmoothedBuffer() const { return buffer.getReadPointer (0); }

private:
    juce::AudioBuffer<FloatType> buffer;
    juce::SmoothedValue<FloatType, ValueSmoothingType> smoother;

    std::atomic<float>* parameterHandle = nullptr;

    double sampleRate = 48000.0;
    double rampLengthInSeconds = 0.05;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SmoothedBufferValue)
};
} // namespace chowdsp
