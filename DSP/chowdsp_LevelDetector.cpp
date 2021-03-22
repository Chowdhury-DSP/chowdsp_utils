namespace chowdsp
{
inline float calcTimeConstant (float timeMs, float expFactor)
{
    return timeMs < 1.0e-3f ? 0.0f : 1.0f - std::exp (expFactor / timeMs);
}

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

    absBuffer.setSize ((int) spec.numChannels, (int) spec.maximumBlockSize);
    absBlock = juce::dsp::AudioBlock<SampleType> { absBuffer };

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
    absBlock.copyFrom (inputBlock);

    if (numInputChannels == 1)
    {
        auto* absPtr = absBlock.getChannelPointer (0);
        juce::FloatVectorOperations::abs (absPtr, absPtr, (int) numSamples);
        outputBlock.copyFrom (absBlock);
    }
    else // sum to mono
    {
        const auto gain = (SampleType) 1.0 / (SampleType) numInputChannels;

        auto* basePtr = absBlock.getChannelPointer (0);
        juce::FloatVectorOperations::abs (basePtr, basePtr, (int) numSamples);
        juce::FloatVectorOperations::copyWithMultiply (levelPtr, basePtr, gain, (int) numSamples);

        for (size_t ch = 1; ch < numInputChannels; ch++)
        {
            auto* otherPtr = absBlock.getChannelPointer (ch);
            juce::FloatVectorOperations::abs (otherPtr, otherPtr, (int) numSamples);
            juce::FloatVectorOperations::addWithMultiply (levelPtr, otherPtr, gain, (int) numSamples);
        }
    }

    for (size_t n = 0; n < numSamples; ++n)
        levelPtr[n] = processSample (levelPtr[n]);
}

} // namespace chowdsp
