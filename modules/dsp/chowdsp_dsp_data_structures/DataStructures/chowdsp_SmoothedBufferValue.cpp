#include "chowdsp_SmoothedBufferValue.h"

namespace chowdsp
{
template <typename FloatType, typename ValueSmoothingTypes>
void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::setParameterHandle (std::atomic<float>* handle)
{
#if JUCE_MODULE_AVAILABLE_chowdsp_parameters
    modulatableParameterHandle = nullptr;
#endif

    parameterHandle = handle;
    reset (parameterHandle->load());
}

#if JUCE_MODULE_AVAILABLE_chowdsp_parameters
template <typename FloatType, typename ValueSmoothingTypes>
void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::setParameterHandle (FloatParameter* handle)
{
    parameterHandle = nullptr;

    modulatableParameterHandle = handle;
    reset (modulatableParameterHandle->getCurrentValue());
}
#endif

template <typename FloatType, typename ValueSmoothingTypes>
void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::prepare (double fs, int samplesPerBlock)
{
    sampleRate = fs;
    buffer.resize ((size_t) samplesPerBlock, {});

    if (parameterHandle != nullptr)
        reset (parameterHandle->load());
#if JUCE_MODULE_AVAILABLE_chowdsp_parameters
    else if (modulatableParameterHandle != nullptr)
        reset (modulatableParameterHandle->getCurrentValue());
#endif
    else
        reset();
}

template <typename FloatType, typename ValueSmoothingTypes>
void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::reset (FloatType resetValue)
{
    smoother.setTargetValue (mappingFunction (resetValue));
    reset();
}

template <typename FloatType, typename ValueSmoothingTypes>
void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::reset()
{
    smoother.reset (sampleRate, rampLengthInSeconds);
    isCurrentlySmoothing = false;
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
    if (parameterHandle != nullptr)
    {
        process ((FloatType) parameterHandle->load(), numSamples);
    }
#if JUCE_MODULE_AVAILABLE_chowdsp_parameters
    else if (modulatableParameterHandle != nullptr)
    {
        process ((FloatType) modulatableParameterHandle->getCurrentValue(), numSamples);
    }
#endif
    else
    {
        // you must set a parameter handle that is not nullptr using setParameterHandle
        // before calling the method!
        jassertfalse;
    }
}

template <typename FloatType, typename ValueSmoothingTypes>
void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::process (FloatType value, int numSamples)
{
    const auto mappedValue = mappingFunction (value);
    smoother.setTargetValue (mappedValue);

    auto* bufferData = buffer.data();
    if (! smoother.isSmoothing())
    {
        isCurrentlySmoothing = false;
        juce::FloatVectorOperations::fill (bufferData, mappedValue, numSamples);
        return;
    }

    isCurrentlySmoothing = true;
    for (int n = 0; n < numSamples; ++n)
        bufferData[n] = smoother.getNextValue();
}

template class SmoothedBufferValue<float, juce::ValueSmoothingTypes::Linear>;
template class SmoothedBufferValue<double, juce::ValueSmoothingTypes::Linear>;
template class SmoothedBufferValue<float, juce::ValueSmoothingTypes::Multiplicative>;
template class SmoothedBufferValue<double, juce::ValueSmoothingTypes::Multiplicative>;
} // namespace chowdsp
