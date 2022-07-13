#include "chowdsp_LevelDetector.h"

namespace chowdsp
{
#ifndef DOXYGEN
inline float calcTimeConstant (float timeMs, float expFactor)
{
    return timeMs < 1.0e-3f ? 0.0f : 1.0f - std::exp (expFactor / timeMs);
}
#endif

template <typename SampleType>
void LevelDetector<SampleType>::setParameters (float attackTimeMs, float releaseTimeMs)
{
    tauAtt = (SampleType) calcTimeConstant (attackTimeMs, expFactor);
    tauRel = (SampleType) calcTimeConstant (releaseTimeMs, expFactor);
}

template <typename SampleType>
void LevelDetector<SampleType>::prepare (const juce::dsp::ProcessSpec& spec)
{
    expFactor = -1000.0f / (float) spec.sampleRate;

    absBuffer.setMaxSize ((int) spec.numChannels, (int) spec.maximumBlockSize);

    reset();
}

template <typename SampleType>
void LevelDetector<SampleType>::reset()
{
    yOld = (SampleType) 0;
    increasing = true;
}

template <typename SampleType>
template <typename ProcessContext>
void LevelDetector<SampleType>::process (const ProcessContext& context) noexcept
{
    const auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();

    const auto numSamples = inputBlock.getNumSamples();
    const auto numInputChannels = inputBlock.getNumChannels();
    jassert (outputBlock.getNumChannels() == 1);

    // take absolute value and sum to mono
    auto* levelPtr = outputBlock.getChannelPointer (0);
    absBuffer.setCurrentSize ((int) numInputChannels, (int) numSamples);
    for (int ch = 0; ch < (int) numInputChannels; ++ch)
        juce::FloatVectorOperations::copy (absBuffer.getWritePointer (ch), inputBlock.getChannelPointer ((size_t) ch), numSamples);

    if (numInputChannels == 1)
    {
        auto* absPtr = absBuffer.getWritePointer (0);
        juce::FloatVectorOperations::abs (absPtr, absPtr, (int) numSamples);

        auto&& outBufferView = BufferView<SampleType> { outputBlock };
        BufferMath::copyBufferData (absBuffer, outBufferView, 0, 0, (int) numSamples);
    }
    else // sum to mono
    {
        const auto gain = (SampleType) 1.0 / (SampleType) numInputChannels;

        auto* basePtr = absBuffer.getWritePointer (0);
        juce::FloatVectorOperations::abs (basePtr, basePtr, (int) numSamples);
        juce::FloatVectorOperations::copyWithMultiply (levelPtr, basePtr, gain, (int) numSamples);

        for (int ch = 1; ch < (int) numInputChannels; ch++)
        {
            auto* otherPtr = absBuffer.getWritePointer (ch);
            juce::FloatVectorOperations::abs (otherPtr, otherPtr, (int) numSamples);
            juce::FloatVectorOperations::addWithMultiply (levelPtr, otherPtr, gain, (int) numSamples);
        }
    }

    for (size_t n = 0; n < numSamples; ++n)
        levelPtr[n] = processSample (levelPtr[n]);
}

} // namespace chowdsp
