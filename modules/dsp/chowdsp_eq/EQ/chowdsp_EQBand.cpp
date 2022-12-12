namespace chowdsp::EQ
{
template <typename FloatType, typename... FilterChoices>
EQBand<FloatType, FilterChoices...>::EQBand() = default;

template <typename FloatType, typename... FilterChoices>
void EQBand<FloatType, FilterChoices...>::setCutoffFrequency (NumericType newCutoffHz)
{
    freqHzHandle = newCutoffHz;
}

template <typename FloatType, typename... FilterChoices>
void EQBand<FloatType, FilterChoices...>::setQValue (NumericType newQValue)
{
    qHandle = newQValue;
}

template <typename FloatType, typename... FilterChoices>
void EQBand<FloatType, FilterChoices...>::setGain (NumericType newGain)
{
    gainHandle = newGain;
}

template <typename FloatType, typename... FilterChoices>
void EQBand<FloatType, FilterChoices...>::setGainDB (NumericType newGainDB)
{
    gainHandle = juce::Decibels::decibelsToGain (newGainDB);
}

template <typename FloatType, typename... FilterChoices>
void EQBand<FloatType, FilterChoices...>::setFilterType (int newFilterType)
{
    jassert (juce::isPositiveAndBelow (newFilterType, numFilterChoices));
    filterType = newFilterType;
}

template <typename FloatType, typename... FilterChoices>
void EQBand<FloatType, FilterChoices...>::prepare (const juce::dsp::ProcessSpec& spec)
{
    fs = (NumericType) spec.sampleRate;

    fadeBuffer.setMaxSize ((int) spec.numChannels, (int) spec.maximumBlockSize);
    fadeBuffer.clear();

    TupleHelpers::forEachInTuple (
        [spec] (auto& filter, size_t)
        {
            using FilterType = std::remove_reference_t<decltype (filter)>;

            if constexpr (std::is_base_of_v<IIRFilter<FilterType::Order, FloatType>, FilterType> || std::is_base_of_v<SOSFilter<FilterType::Order, FloatType>, FilterType> || std::is_base_of_v<SOSFilter<FilterType::Order - 1, FloatType>, FilterType>)
                filter.prepare ((int) spec.numChannels);
            else if constexpr (std::is_same_v<StateVariableFilter<FloatType, FilterType::Type>, FilterType> || std::is_same_v<NthOrderFilter<FloatType, FilterType::Order, FilterType::Type>, FilterType>)
                filter.prepare (spec);
            else
                jassertfalse; // unknown filter type!
        },
        filters);

    for (auto* smoother : { &freqSmooth, &qSmooth, &gainSmooth })
    {
        smoother->prepare (spec.sampleRate, (int) spec.maximumBlockSize);
        smoother->setRampLength (0.05);
    }

    reset();
}

template <typename FloatType, typename... FilterChoices>
void EQBand<FloatType, FilterChoices...>::reset()
{
    TupleHelpers::forEachInTuple ([] (auto& filter, size_t)
                                  { filter.reset(); },
                                  filters);

    for (auto* smoother : { &freqSmooth, &qSmooth, &gainSmooth })
        smoother->reset();

    prevFilterType = filterType;
}

template <typename FloatType, typename... FilterChoices>
template <typename FilterType, typename T, size_t N>
std::enable_if_t<std::is_base_of_v<IIRFilter<N, T>, FilterType> || std::is_base_of_v<SOSFilter<N, T>, FilterType> || std::is_base_of_v<SOSFilter<N - 1, T>, FilterType>, void>
    EQBand<FloatType, FilterChoices...>::processFilterChannel (FilterType& filter, const BufferView<FloatType>& block)
{
    auto setParams = [&filter, fs = this->fs] (FloatType curFreq, FloatType curQ, FloatType curGain)
    {
        if constexpr (! FilterType::HasQParameter)
        {
            juce::ignoreUnused (curQ, curGain);
            filter.calcCoefs (curFreq, fs);
        }
        else if constexpr (! FilterType::HasGainParameter)
        {
            juce::ignoreUnused (curGain);
            filter.calcCoefs (curFreq, curQ, fs);
        }
        else
        {
            filter.calcCoefs (curFreq, curQ, curGain, fs);
        }
    };

    const auto isSmoothing = freqSmooth.isSmoothing() || qSmooth.isSmoothing() || gainSmooth.isSmoothing();
    if (isSmoothing)
    {
        filter.processBlockWithModulation (
            block,
            [setParamsFunc = std::forward<decltype (setParams)> (setParams),
             freqHzValues = freqSmooth.getSmoothedBuffer(),
             qValues = qSmooth.getSmoothedBuffer(),
             gainValues = gainSmooth.getSmoothedBuffer()] (int n)
            { setParamsFunc (freqHzValues[n], qValues[n], gainValues[n]); });
    }
    else
    {
        setParams (freqSmooth.getCurrentValue(), qSmooth.getCurrentValue(), gainSmooth.getCurrentValue());
        filter.processBlock (block);
    }
}

template <typename FloatType, typename... FilterChoices>
template <typename FilterType, typename T, StateVariableFilterType type>
std::enable_if_t<std::is_same_v<StateVariableFilter<T, type>, FilterType>, void>
    EQBand<FloatType, FilterChoices...>::processFilterChannel (FilterType& filter, const BufferView<FloatType>& block)
{
    const auto numChannels = (int) block.getNumChannels();
    const auto numSamples = (int) block.getNumSamples();

    const auto* freqHzValues = freqSmooth.getSmoothedBuffer();
    const auto* qValues = qSmooth.getSmoothedBuffer();
    const auto* gainValues = gainSmooth.getSmoothedBuffer();

    auto blockData = block.getArrayOfWritePointers();
    if (freqSmooth.isSmoothing() && qSmooth.isSmoothing() && gainSmooth.isSmoothing())
    {
        for (int n = 0; n < numSamples; ++n)
        {
            filter.template setCutoffFrequency<false> (freqHzValues[n]);
            filter.template setQValue<false> (qValues[n]);
            filter.template setGain<false> (gainValues[n]);
            filter.update();
            for (int ch = 0; ch < numChannels; ++ch)
                blockData[ch][n] = filter.processSample (ch, blockData[ch][n]);
        }
    }
    else if (freqSmooth.isSmoothing() && qSmooth.isSmoothing())
    {
        for (int n = 0; n < numSamples; ++n)
        {
            filter.template setCutoffFrequency<false> (freqHzValues[n]);
            filter.template setQValue<false> (qValues[n]);
            filter.update();
            for (int ch = 0; ch < numChannels; ++ch)
                blockData[ch][n] = filter.processSample (ch, blockData[ch][n]);
        }
    }
    else if (freqSmooth.isSmoothing() && gainSmooth.isSmoothing())
    {
        for (int n = 0; n < numSamples; ++n)
        {
            filter.template setCutoffFrequency<false> (freqHzValues[n]);
            filter.template setGain<false> (gainValues[n]);
            filter.update();
            for (int ch = 0; ch < numChannels; ++ch)
                blockData[ch][n] = filter.processSample (ch, blockData[ch][n]);
        }
    }
    else if (qSmooth.isSmoothing() && gainSmooth.isSmoothing())
    {
        for (int n = 0; n < numSamples; ++n)
        {
            filter.template setQValue<false> (qValues[n]);
            filter.template setGain<false> (gainValues[n]);
            filter.update();
            for (int ch = 0; ch < numChannels; ++ch)
                blockData[ch][n] = filter.processSample (ch, blockData[ch][n]);
        }
    }
    else if (freqSmooth.isSmoothing())
    {
        for (int n = 0; n < numSamples; ++n)
        {
            filter.setCutoffFrequency (freqHzValues[n]);
            for (int ch = 0; ch < numChannels; ++ch)
                blockData[ch][n] = filter.processSample (ch, blockData[ch][n]);
        }
    }
    else if (qSmooth.isSmoothing())
    {
        for (int n = 0; n < numSamples; ++n)
        {
            filter.setQValue (qValues[n]);
            for (int ch = 0; ch < numChannels; ++ch)
                blockData[ch][n] = filter.processSample (ch, blockData[ch][n]);
        }
    }
    else if (gainSmooth.isSmoothing())
    {
        for (int n = 0; n < numSamples; ++n)
        {
            filter.setGain (gainValues[n]);
            for (int ch = 0; ch < numChannels; ++ch)
                blockData[ch][n] = filter.processSample (ch, blockData[ch][n]);
        }
    }
    else
    {
        filter.template setCutoffFrequency<false> (freqSmooth.getCurrentValue());
        filter.template setQValue<false> (qSmooth.getCurrentValue());
        filter.template setGain<false> (gainSmooth.getCurrentValue());
        filter.update();
        filter.processBlock (block);
    }
}

template <typename FloatType, typename... FilterChoices>
template <typename FilterType, typename T, size_t N, StateVariableFilterType type>
std::enable_if_t<std::is_same_v<NthOrderFilter<T, N, type>, FilterType>, void>
    EQBand<FloatType, FilterChoices...>::processFilterChannel (FilterType& filter, const BufferView<FloatType>& block)
{
    const auto numChannels = (int) block.getNumChannels();
    const auto numSamples = (int) block.getNumSamples();

    const auto* freqHzValues = freqSmooth.getSmoothedBuffer();
    const auto* qValues = qSmooth.getSmoothedBuffer();

    auto blockData = block.getArrayOfWritePointers();
    if (freqSmooth.isSmoothing() && qSmooth.isSmoothing())
    {
        for (int n = 0; n < numSamples; ++n)
        {
            filter.setCutoffFrequency (freqHzValues[n]);
            filter.setQValue (qValues[n]);
            for (int ch = 0; ch < numChannels; ++ch)
                blockData[ch][n] = filter.processSample (ch, blockData[ch][n]);
        }
    }
    else if (freqSmooth.isSmoothing())
    {
        for (int n = 0; n < numSamples; ++n)
        {
            filter.setCutoffFrequency (freqHzValues[n]);
            for (int ch = 0; ch < numChannels; ++ch)
                blockData[ch][n] = filter.processSample (ch, blockData[ch][n]);
        }
    }
    else if (qSmooth.isSmoothing())
    {
        for (int n = 0; n < numSamples; ++n)
        {
            filter.setQValue (qValues[n]);
            for (int ch = 0; ch < numChannels; ++ch)
                blockData[ch][n] = filter.processSample (ch, blockData[ch][n]);
        }
    }
    else
    {
        filter.setCutoffFrequency (freqSmooth.getCurrentValue());
        filter.setQValue (qSmooth.getCurrentValue());
        filter.processBlock (block);
    }
}

template <typename FloatType, typename... FilterChoices>
void EQBand<FloatType, FilterChoices...>::fadeBuffers (const FloatType* fadeInBuffer, const FloatType* fadeOutBuffer, FloatType* targetBuffer, int numSamples)
{
    auto fadeInGain = (FloatType) 0;
    auto fadeOutGain = (FloatType) 1;
    const auto increment = (FloatType) 1 / (FloatType) numSamples;
    for (int n = 0; n < numSamples; ++n)
    {
        targetBuffer[n] = fadeInGain * fadeInBuffer[n] + fadeOutGain * fadeOutBuffer[n];
        fadeInGain += increment;
        fadeOutGain -= increment;
    }
}

template <typename FloatType, typename... FilterChoices>
void EQBand<FloatType, FilterChoices...>::processBlock (const BufferView<FloatType>& buffer) noexcept
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    freqSmooth.process (freqHzHandle, numSamples);
    qSmooth.process (qHandle, numSamples);
    gainSmooth.process (gainHandle, numSamples);

    const auto needsFade = filterType != prevFilterType;
    if (needsFade)
        BufferMath::copyBufferData (buffer, fadeBuffer, 0, 0, numSamples, 0, numChannels);

    TupleHelpers::forEachInTuple (
        [this, &buffer] (auto& filter, size_t filterIndex)
        {
            if ((int) filterIndex == filterType)
            {
                processFilterChannel (filter, buffer);
            }
            else if ((int) filterIndex == prevFilterType)
            {
                processFilterChannel (filter, fadeBuffer);
                filter.reset();
            }
        },
        filters);

    if (needsFade)
    {
        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* blockPtr = buffer.getWritePointer (channel);
            fadeBuffers (blockPtr, fadeBuffer.getReadPointer (channel), blockPtr, numSamples);
        }
    }

    prevFilterType = filterType;
}

template <typename FloatType, typename... FilterChoices>
template <typename ProcessContext>
void EQBand<FloatType, FilterChoices...>::process (const ProcessContext& context) noexcept
{
    const auto& inputBlock = context.getInputBlock();
    auto& block = context.getOutputBlock();
    const auto numChannels = block.getNumChannels();
    const auto numSamples = (int) block.getNumSamples();

    jassert (inputBlock.getNumChannels() == numChannels);
    jassert (inputBlock.getNumSamples() == (size_t) numSamples);

    freqSmooth.process (freqHzHandle, numSamples);
    qSmooth.process (qHandle, numSamples);
    gainSmooth.process (gainHandle, numSamples);

    // the filters will need to do in-place processing anyway, so let's just copy the blocks here
    if (context.usesSeparateInputAndOutputBlocks())
        block.copyFrom (inputBlock);

    if (context.isBypassed)
    {
        reset();
        return;
    }

    const auto needsFade = filterType != prevFilterType;
    if (needsFade)
        BufferMath::copyBufferData (block, fadeBuffer, 0, numSamples, 0, numChannels);

    TupleHelpers::forEachInTuple (
        [this, &block] (auto& filter, size_t filterIndex)
        {
            if ((int) filterIndex == filterType)
            {
                processFilterChannel (filter, block);
            }
            else if ((int) filterIndex == prevFilterType)
            {
                processFilterChannel (filter, fadeBuffer);
                filter.reset();
            }
        },
        filters);

    if (needsFade)
    {
        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            auto* blockPtr = block.getChannelPointer (channel);
            fadeBuffers (blockPtr, fadeBuffer.getReadPointer ((int) channel), blockPtr, numSamples);
        }
    }

    prevFilterType = filterType;
}

} // namespace chowdsp::EQ
