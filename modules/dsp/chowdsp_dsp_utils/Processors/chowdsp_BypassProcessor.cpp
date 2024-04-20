namespace chowdsp
{
template <typename SampleType, typename DelayInterpType>
void BypassProcessor<SampleType,
                     DelayInterpType,
                     std::enable_if_t<std::is_same_v<DelayInterpType, NullType>>>::prepare (const juce::dsp::ProcessSpec& spec, bool onOffParam, bool useInternalBuffer)
{
    prevOnOffParam = onOffParam;
    if (useInternalBuffer)
        fadeBuffer.setMaxSize ((int) spec.numChannels, (int) spec.maximumBlockSize);
}

template <typename SampleType, typename DelayInterpType>
bool BypassProcessor<SampleType,
                     DelayInterpType,
                     std::enable_if_t<std::is_same_v<DelayInterpType, NullType>>>::processBlockIn (const BufferView<const SampleType>& block,
                                                                                                   bool onOffParam,
                                                                                                   std::optional<ArenaAllocatorView> arena)
{
    if (! onOffParam && ! prevOnOffParam)
    {
        return false;
    }

    if (onOffParam != prevOnOffParam)
    {
        if (arena.has_value())
            fadeBufferView = make_temp_buffer<SampleType> (*arena, block.getNumChannels(), block.getNumSamples());
        else
            fadeBufferView = BufferView { fadeBuffer, 0, block.getNumSamples(), 0, block.getNumChannels() };

        BufferMath::copyBufferData (block, fadeBufferView);
    }

    return true;
}

template <typename SampleType, typename DelayInterpType>
void BypassProcessor<SampleType,
                     DelayInterpType,
                     std::enable_if_t<std::is_same_v<DelayInterpType, NullType>>>::processBlockOut (const BufferView<SampleType>& block, bool onOffParam)
{
    auto fadeOutputBuffer = [onOffParam] (auto* blockPtr, const auto* fadePtr, const int startSample, const int numSamples)
    {
        SampleType startGain = ! onOffParam ? static_cast<NumericType> (1) // fade out
                                            : static_cast<NumericType> (0); // fade in
        SampleType endGain = static_cast<NumericType> (1) - startGain;

        SampleType gain = startGain;
        SampleType increment = (endGain - startGain) / (NumericType) (numSamples - startSample);

        // From samples [0, startSample)
        if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
        {
            const auto fadeBufferGain = static_cast<NumericType> (1) - gain;
            for (int n = 0; n < startSample; ++n)
                blockPtr[n] = blockPtr[n] * gain + fadePtr[n] * fadeBufferGain;
        }
        else
        {
            juce::FloatVectorOperations::multiply (blockPtr, gain, startSample);
            juce::FloatVectorOperations::addWithMultiply (blockPtr, fadePtr, static_cast<SampleType> (1) - gain, startSample);
        }

        // From [startSample, numSamples)
        for (int n = startSample; n < numSamples; ++n)
        {
            blockPtr[n] = blockPtr[n] * gain + fadePtr[n] * (static_cast<NumericType> (1) - gain);
            gain += increment;
        }
    };

    if (onOffParam == prevOnOffParam)
    {
        return;
    }

    const auto numChannels = block.getNumChannels();
    const auto numSamples = block.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* blockPtr = block.getWritePointer (ch);
        const auto* fadePtr = fadeBufferView.getReadPointer (ch);

        fadeOutputBuffer (blockPtr, fadePtr, 0, numSamples);
    }

    if (0 < numSamples)
        prevOnOffParam = onOffParam;
}

//===========================================================
template <typename SampleType, typename DelayInterpType>
BypassProcessor<SampleType,
                DelayInterpType,
                std::enable_if_t<! std::is_same_v<DelayInterpType, NullType>>>::BypassProcessor (int maxLatencySamples)
    : maximumLatencySamples (maxLatencySamples)
{
}

template <typename SampleType, typename DelayInterpType>
void BypassProcessor<SampleType,
                     DelayInterpType,
                     std::enable_if_t<! std::is_same_v<DelayInterpType, NullType>>>::prepare (const juce::dsp::ProcessSpec& spec,
                                                                                              bool onOffParam,
                                                                                              bool useInternalBuffer)
{
    prevOnOffParam = onOffParam;
    if (useInternalBuffer)
        fadeBuffer.setMaxSize ((int) spec.numChannels, (int) spec.maximumBlockSize);

    compDelay.emplace (maximumLatencySamples + static_cast<int> (spec.maximumBlockSize));
    compDelay->prepare (spec); // sample rate does not matter
}

template <typename SampleType, typename DelayInterpType>
void BypassProcessor<SampleType,
                     DelayInterpType,
                     std::enable_if_t<! std::is_same_v<DelayInterpType, NullType>>>::setLatencySamples (int delaySamples)
{
    setLatencySamplesInternal ((NumericType) delaySamples);
}

template <typename SampleType, typename DelayInterpType>
void BypassProcessor<SampleType,
                     DelayInterpType,
                     std::enable_if_t<! std::is_same_v<DelayInterpType, NullType>>>::setLatencySamples (NumericType delaySamples)
{
    static_assert (! std::is_same_v<DelayInterpType, DelayLineInterpolationTypes::None>, "Attempting to set non-integer latency value without using delay interpolation!");
    setLatencySamplesInternal (delaySamples);
}

template <typename SampleType, typename DelayInterpType>
void BypassProcessor<SampleType,
                     DelayInterpType,
                     std::enable_if_t<! std::is_same_v<DelayInterpType, NullType>>>::setLatencySamplesInternal (NumericType delaySamples)
{
    if (juce::approximatelyEqual (delaySamples, prevDelay))
        return;

    compDelay->setDelay ((NumericType) delaySamples);

    if (juce::approximatelyEqual (delaySamples, (NumericType) 0))
        compDelay->reset();

    prevDelay = delaySamples;
}

template <typename SampleType, typename DelayInterpType>
bool BypassProcessor<SampleType,
                     DelayInterpType,
                     std::enable_if_t<! std::is_same_v<DelayInterpType, NullType>>>::processBlockIn (const BufferView<SampleType>& block,
                                                                                                     bool onOffParam,
                                                                                                     std::optional<ArenaAllocatorView> arena)
{
    enum class DelayOp
    {
        Pop,
        Push,
        Toss,
    };

    auto doDelayOp = [] (auto& sampleBuffer, auto& delay, DelayOp op)
    {
        if (juce::approximatelyEqual (delay.getDelay(), NumericType (0)))
            return;

        for (int ch = 0; ch < sampleBuffer.getNumChannels(); ++ch)
        {
            if (op == DelayOp::Push)
            {
                auto* x = sampleBuffer.getReadPointer (ch);
                for (int n = 0; n < sampleBuffer.getNumSamples(); ++n)
                    delay.pushSample (ch, x[n]);
            }
            else if (op == DelayOp::Pop)
            {
                auto* x = sampleBuffer.getWritePointer (ch);
                for (int n = 0; n < sampleBuffer.getNumSamples(); ++n)
                    x[n] = delay.popSample (ch);
            }
            else if (op == DelayOp::Toss)
            {
                for (int n = 0; n < sampleBuffer.getNumSamples(); ++n)
                    delay.incrementReadPointer (ch);
            }
        }
    };

    doDelayOp (block, *compDelay, DelayOp::Push);

    if (! onOffParam && ! prevOnOffParam)
    {
        doDelayOp (block, *compDelay, DelayOp::Pop);
        return false;
    }

    if (onOffParam != prevOnOffParam)
    {
        if (arena.has_value())
            fadeBufferView = make_temp_buffer<SampleType> (*arena, block.getNumChannels(), block.getNumSamples());
        else
            fadeBufferView = BufferView { fadeBuffer, 0, block.getNumSamples(), 0, block.getNumChannels() };

        BufferMath::copyBufferData (block, fadeBufferView);
        doDelayOp (fadeBufferView, *compDelay, DelayOp::Pop);

        if (onOffParam && latencySampleCount < 0)
            latencySampleCount = (int) compDelay->getDelay();
    }
    else
    {
        doDelayOp (block, *compDelay, DelayOp::Toss);
    }

    return true;
}

template <typename SampleType, typename DelayInterpType>
void BypassProcessor<SampleType,
                     DelayInterpType,
                     std::enable_if_t<! std::is_same_v<DelayInterpType, NullType>>>::processBlockOut (const BufferView<SampleType>& block, bool onOffParam)
{
    auto fadeOutputBuffer = [onOffParam] (auto* blockPtr, const auto* fadePtr, const int startSample, const int numSamples)
    {
        SampleType startGain = ! onOffParam ? static_cast<NumericType> (1) // fade out
                                            : static_cast<NumericType> (0); // fade in
        SampleType endGain = static_cast<NumericType> (1) - startGain;

        SampleType gain = startGain;
        SampleType increment = (endGain - startGain) / (NumericType) (numSamples - startSample);

        // From samples [0, startSample)
        if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
        {
            const auto fadeBufferGain = static_cast<NumericType> (1) - gain;
            for (int n = 0; n < startSample; ++n)
                blockPtr[n] = blockPtr[n] * gain + fadePtr[n] * fadeBufferGain;
        }
        else
        {
            juce::FloatVectorOperations::multiply (blockPtr, gain, startSample);
            juce::FloatVectorOperations::addWithMultiply (blockPtr, fadePtr, static_cast<SampleType> (1) - gain, startSample);
        }

        // From [startSample, numSamples)
        for (int n = startSample; n < numSamples; ++n)
        {
            blockPtr[n] = blockPtr[n] * gain + fadePtr[n] * (static_cast<NumericType> (1) - gain);
            gain += increment;
        }
    };

    if (onOffParam == prevOnOffParam)
    {
        latencySampleCount = 0;
        return;
    }

    const auto numChannels = block.getNumChannels();
    const auto numSamples = block.getNumSamples();
    const auto startSample = getFadeStartSample (numSamples);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* blockPtr = block.getWritePointer (ch);
        const auto* fadePtr = fadeBufferView.getReadPointer (ch);

        fadeOutputBuffer (blockPtr, fadePtr, startSample, numSamples);
    }

    if (startSample < numSamples)
        prevOnOffParam = onOffParam;
}

template <typename SampleType, typename DelayInterpType>
int BypassProcessor<SampleType,
                    DelayInterpType,
                    std::enable_if_t<! std::is_same_v<DelayInterpType, NullType>>>::getFadeStartSample (const int numSamples)
{
    if (latencySampleCount <= 0)
    {
        latencySampleCount = -1;
        return 0;
    }

    if (latencySampleCount < numSamples / 4) // small offset is okay, just adjust the fade start
    {
        int startSample = latencySampleCount;
        latencySampleCount = -1;
        return startSample;
    }
    else // wait for latency time to catch up
    {
        latencySampleCount = juce::jmax (latencySampleCount - numSamples, 0);
        return numSamples;
    }
}
} // namespace chowdsp
