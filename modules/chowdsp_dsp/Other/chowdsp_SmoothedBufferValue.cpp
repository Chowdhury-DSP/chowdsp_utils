#include "chowdsp_SmoothedBufferValue.h"

namespace chowdsp
{
template <typename FloatType, typename ValueSmoothingTypes>
void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::setParameterHandle (std::atomic<float>* handle)
{
    parameterHandle = handle;
    reset (*parameterHandle);
}

template <typename FloatType, typename ValueSmoothingTypes>
void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::prepare (double fs, int samplesPerBlock)
{
    sampleRate = fs;
    buffer.setSize (1, samplesPerBlock);

    if (parameterHandle != nullptr)
        reset (*parameterHandle);
    else
        reset();
}

template <typename FloatType, typename ValueSmoothingTypes>
void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::reset (FloatType resetValue)
{
    smoother.setTargetValue (resetValue);
    reset();
}

template <typename FloatType, typename ValueSmoothingTypes>
void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::reset()
{
    smoother.reset (sampleRate, rampLengthInSeconds);
}

template <typename FloatType, typename ValueSmoothingTypes>
void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::setRampLength (double rampLengthSeconds)
{
    rampLengthInSeconds = rampLengthSeconds;
    reset();
}

template <typename FloatType, typename ValueSmoothingTypes>
void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::process (int numSamples)
{
    jassert (parameterHandle != nullptr);
    process (*parameterHandle, numSamples);
}

template <typename FloatType, typename ValueSmoothingTypes>
void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::process (FloatType value, int numSamples)
{
    smoother.setTargetValue (value);

    auto* bufferData = buffer.getWritePointer (0);
    if (! smoother.isSmoothing())
    {
        juce::FloatVectorOperations::fill (bufferData, value, numSamples);
        return;
    }

    for (int n = 0; n < numSamples; ++n)
        bufferData[n] = smoother.getNextValue();
}

template class SmoothedBufferValue<float, juce::ValueSmoothingTypes::Linear>;
template class SmoothedBufferValue<double, juce::ValueSmoothingTypes::Linear>;
template class SmoothedBufferValue<float, juce::ValueSmoothingTypes::Multiplicative>;
template class SmoothedBufferValue<double, juce::ValueSmoothingTypes::Multiplicative>;
} // namespace chowdsp
