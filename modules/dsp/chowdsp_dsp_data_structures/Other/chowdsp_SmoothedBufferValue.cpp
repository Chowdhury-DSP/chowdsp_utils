#include "chowdsp_SmoothedBufferValue.h"

namespace chowdsp
{
template <typename FloatType, typename ValueSmoothingTypes>
void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::setParameterHandle (std::atomic<float>* handle)
{
    modulatableParameterHandle = nullptr;

    parameterHandle = handle;
    reset (parameterHandle->load());
}

template <typename FloatType, typename ValueSmoothingTypes>
void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::setParameterHandle ([[maybe_unused]] const FloatParameter* handle)
{
    parameterHandle = nullptr;

#if JUCE_MODULE_AVAILABLE_chowdsp_parameters
    modulatableParameterHandle = handle;
    reset (modulatableParameterHandle->getCurrentValue());
#endif
}

template <typename FloatType, typename ValueSmoothingTypes>
void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::prepare (double fs, int samplesPerBlock, bool useInternalVector)
{
    sampleRate = fs;
    if (useInternalVector)
    {
        buffer.resize ((size_t) samplesPerBlock, {});
        bufferData = buffer.data();
    }
    smoother.reset (sampleRate, rampLengthInSeconds);

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
    smoother.setCurrentAndTargetValue (mappingFunction (resetValue));
    isCurrentlySmoothing = false;
}

template <typename FloatType, typename ValueSmoothingTypes>
void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::reset()
{
    if (parameterHandle != nullptr)
    {
        reset ((FloatType) parameterHandle->load());
    }
#if JUCE_MODULE_AVAILABLE_chowdsp_parameters
    else if (modulatableParameterHandle != nullptr)
    {
        reset ((FloatType) modulatableParameterHandle->getCurrentValue());
    }
#endif
    else
    {
        reset (getCurrentValue());
    }
}

template <typename FloatType, typename ValueSmoothingTypes>
void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::setRampLength (double rampLengthSeconds)
{
    rampLengthInSeconds = rampLengthSeconds;
    reset();
    smoother.reset (sampleRate, rampLengthInSeconds);
    isCurrentlySmoothing = false;
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
void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::process (int numSamples, ArenaAllocatorView alloc)
{
    bufferData = alloc.allocate<FloatType> (numSamples, bufferAlignment);
    jassert (bufferData != nullptr); // arena allocator is out of memory!
    process (numSamples);
}

template <typename FloatType, typename ValueSmoothingTypes>
void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::process (FloatType value, int numSamples)
{
    const auto mappedValue = mappingFunction (value);
    smoother.setTargetValue (mappedValue);

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

template <typename FloatType, typename ValueSmoothingTypes>
void SmoothedBufferValue<FloatType, ValueSmoothingTypes>::process (FloatType value, int numSamples, ArenaAllocatorView alloc)
{
    bufferData = alloc.allocate<FloatType> (numSamples, bufferAlignment);
    jassert (bufferData != nullptr); // arena allocator is out of memory!
    process (value, numSamples);
}

template class SmoothedBufferValue<float, juce::ValueSmoothingTypes::Linear>;
template class SmoothedBufferValue<double, juce::ValueSmoothingTypes::Linear>;
template class SmoothedBufferValue<float, juce::ValueSmoothingTypes::Multiplicative>;
template class SmoothedBufferValue<double, juce::ValueSmoothingTypes::Multiplicative>;
} // namespace chowdsp
