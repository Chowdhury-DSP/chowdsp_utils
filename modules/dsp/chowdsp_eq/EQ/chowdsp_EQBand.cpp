namespace chowdsp::EQ
{
template <typename FloatType, typename FilterChoicesTuple>
EQBandBase<FloatType, FilterChoicesTuple>::EQBandBase() = default;

template <typename FloatType, typename FilterChoicesTuple>
void EQBandBase<FloatType, FilterChoicesTuple>::setCutoffFrequency (NumericType newCutoffHz)
{
    freqHzHandle = newCutoffHz;
}

template <typename FloatType, typename FilterChoicesTuple>
void EQBandBase<FloatType, FilterChoicesTuple>::setQValue (NumericType newQValue)
{
    qHandle = newQValue;
}

template <typename FloatType, typename FilterChoicesTuple>
void EQBandBase<FloatType, FilterChoicesTuple>::setGain (NumericType newGain)
{
    gainHandle = newGain;
}

template <typename FloatType, typename FilterChoicesTuple>
void EQBandBase<FloatType, FilterChoicesTuple>::setGainDB (NumericType newGainDB)
{
    gainHandle = juce::Decibels::decibelsToGain (newGainDB);
}

template <typename FloatType, typename FilterChoicesTuple>
void EQBandBase<FloatType, FilterChoicesTuple>::setFilterType (int newFilterType)
{
    jassert (juce::isPositiveAndBelow (newFilterType, numFilterChoices));
    filterType = newFilterType;
}

template <typename FloatType, typename FilterChoicesTuple>
void EQBandBase<FloatType, FilterChoicesTuple>::prepare (const juce::dsp::ProcessSpec& spec)
{
    fs = (NumericType) spec.sampleRate;

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
        smoother->prepare (spec.sampleRate, (int) spec.maximumBlockSize, false);
        smoother->setRampLength (0.05);
    }

    reset();
}

template <typename FloatType, typename FilterChoicesTuple>
void EQBandBase<FloatType, FilterChoicesTuple>::reset()
{
    TupleHelpers::forEachInTuple ([] (auto& filter, size_t)
                                  { filter.reset(); },
                                  filters);

    freqSmooth.reset (freqHzHandle);
    qSmooth.reset (qHandle);
    gainSmooth.reset (gainHandle);

    prevFilterType = filterType;
}

template <typename FloatType, typename FilterChoicesTuple>
template <typename FilterType, typename T, size_t N>
std::enable_if_t<std::is_base_of_v<IIRFilter<N, T>, FilterType> || std::is_base_of_v<SOSFilter<N, T>, FilterType> || std::is_base_of_v<SOSFilter<N - 1, T>, FilterType>, void>
    EQBandBase<FloatType, FilterChoicesTuple>::processFilterChannel (FilterType& filter, const BufferView<FloatType>& block)
{
    const auto setParams = [&filter, fs = this->fs] (FloatType curFreq, FloatType curQ, FloatType curGain)
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
            [&setParams,
             freqHzValues = freqSmooth.getSmoothedBuffer(),
             qValues = qSmooth.getSmoothedBuffer(),
             gainValues = gainSmooth.getSmoothedBuffer()] (int n)
            { setParams (freqHzValues[n], qValues[n], gainValues[n]); });
    }
    else
    {
        setParams (freqSmooth.getCurrentValue(), qSmooth.getCurrentValue(), gainSmooth.getCurrentValue());
        filter.processBlock (block);
    }
}

template <typename FloatType, typename FilterChoicesTuple>
template <typename FilterType, typename T, StateVariableFilterType type>
std::enable_if_t<std::is_same_v<StateVariableFilter<T, type>, FilterType>, void>
    EQBandBase<FloatType, FilterChoicesTuple>::processFilterChannel (FilterType& filter, const BufferView<FloatType>& block)
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

template <typename FloatType, typename FilterChoicesTuple>
template <typename FilterType, typename T, size_t N, StateVariableFilterType type>
std::enable_if_t<std::is_same_v<NthOrderFilter<T, N, type>, FilterType>, void>
    EQBandBase<FloatType, FilterChoicesTuple>::processFilterChannel (FilterType& filter, const BufferView<FloatType>& block)
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

template <typename FloatType, typename FilterChoicesTuple>
void EQBandBase<FloatType, FilterChoicesTuple>::fadeBuffers (const FloatType* fadeInBuffer, const FloatType* fadeOutBuffer, FloatType* targetBuffer, int numSamples) const
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

template <typename FloatType, typename FilterChoicesTuple>
void EQBandBase<FloatType, FilterChoicesTuple>::processBlock (const BufferView<FloatType>& buffer, ArenaAllocatorView arena) noexcept
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();
    const auto frame = arena.create_frame();

    freqSmooth.process (freqHzHandle, numSamples, arena);
    qSmooth.process (qHandle, numSamples, arena);
    gainSmooth.process (gainHandle, numSamples, arena);

    const auto needsFade = filterType != prevFilterType;
    BufferView<FloatType> fadeBuffer {};
    if (needsFade)
    {
        fadeBuffer = make_temp_buffer<FloatType> (arena, numChannels, numSamples);
        BufferMath::copyBufferData (buffer, fadeBuffer);
    }

    TupleHelpers::forEachInTuple (
        [this, &buffer, &fadeBuffer] (auto& filter, size_t filterIndex)
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
} // namespace chowdsp::EQ
