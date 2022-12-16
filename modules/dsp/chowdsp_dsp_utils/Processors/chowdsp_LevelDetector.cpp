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

    absBuffer.setMaxSize (1, (int) spec.maximumBlockSize);

    reset();
}

template <typename SampleType>
void LevelDetector<SampleType>::reset()
{
    yOld = (SampleType) 0;
    increasing = true;
}

template <typename SampleType>
void LevelDetector<SampleType>::processBlock (const BufferView<SampleType>& buffer) noexcept
{
    processBlock (buffer, buffer);
}

template <typename SampleType>
void LevelDetector<SampleType>::processBlock (const BufferView<const SampleType>& bufferIn, const BufferView<SampleType>& bufferOut) noexcept
{
    const auto numSamples = bufferIn.getNumSamples();
    const auto numInputChannels = bufferIn.getNumChannels();
    jassert (bufferOut.getNumSamples() == numSamples);

    auto* levelData = bufferOut.getWritePointer (0);
    if (numInputChannels == 1)
    {
        juce::FloatVectorOperations::abs (levelData, bufferIn.getReadPointer (0), numSamples);
    }
    else
    {
        absBuffer.setCurrentSize (1, numSamples);
        auto* absData = absBuffer.getWritePointer (0);

        juce::FloatVectorOperations::abs (levelData, bufferIn.getReadPointer (0), numSamples);
        for (int ch = 1; ch < numInputChannels; ch++)
        {
            juce::FloatVectorOperations::abs (absData, bufferIn.getReadPointer (ch), numSamples);
            juce::FloatVectorOperations::add (levelData, absData, numSamples);
        }

        const auto normGain = (SampleType) 1.0 / (SampleType) numInputChannels;
        juce::FloatVectorOperations::multiply (levelData, normGain, numSamples);
    }

    ScopedValue _increasing { increasing };
    ScopedValue _yOld { yOld };
    for (int n = 0; n < numSamples; ++n)
        levelData[n] = processSampleInternal (levelData[n], _increasing.get(), _yOld.get());
}

template <typename SampleType>
template <typename ProcessContext>
void LevelDetector<SampleType>::process (const ProcessContext& context) noexcept
{
    const auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();

    processBlock (inputBlock, outputBlock);
}
} // namespace chowdsp
