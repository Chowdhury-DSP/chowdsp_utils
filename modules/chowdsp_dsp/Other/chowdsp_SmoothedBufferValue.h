#pragma once

namespace chowdsp
{
template <typename FloatType, typename ValueSmoothingType = juce::ValueSmoothingTypes::Linear>
class SmoothedBufferValue
{
public:
    SmoothedBufferValue() = default;

    void setParameterHandle (std::atomic<float>* handle) { parameterHandle = handle; }

    void prepare (double sampleRate, int samplesPerBlock);
    void reset();

    void setRampLength (double rampLengthSeconds);
    bool isSmoothing() const noexcept { return smoother.isSmoothing(); }

    void process (int numSamples);
    void process (FloatType value, int numSamples);

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
