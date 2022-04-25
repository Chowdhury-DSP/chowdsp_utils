namespace chowdsp
{
#ifndef DOXYGEN
namespace eqband_detail
{
    /** Functions to do a function for each element in the tuple */
    template <typename Fn, typename Tuple, size_t... Ix>
    constexpr void forEachInTuple (Fn&& fn, Tuple&& tuple, std::index_sequence<Ix...>) noexcept (noexcept (std::initializer_list<int> { (fn (std::get<Ix> (tuple), Ix), 0)... }))
    {
        (void) std::initializer_list<int> { ((void) fn (std::get<Ix> (tuple), Ix), 0)... };
    }

    template <typename T>
    using TupleIndexSequence = std::make_index_sequence<std::tuple_size<std::remove_cv_t<std::remove_reference_t<T>>>::value>;

    template <typename Fn, typename Tuple>
    constexpr void forEachInTuple (Fn&& fn, Tuple&& tuple) noexcept (noexcept (forEachInTuple (std::forward<Fn> (fn), std::forward<Tuple> (tuple), TupleIndexSequence<Tuple> {})))
    {
        forEachInTuple (std::forward<Fn> (fn), std::forward<Tuple> (tuple), TupleIndexSequence<Tuple> {});
    }
} // namespace eqband_detail
#endif

template <typename FloatType, typename... FilterChoices>
EQBand<FloatType, FilterChoices...>::EQBand() = default;

template <typename FloatType, typename... FilterChoices>
void EQBand<FloatType, FilterChoices...>::setCutoffFrequency (FloatType newCutoffHz)
{
    freqHzHandle = newCutoffHz;
}

template <typename FloatType, typename... FilterChoices>
void EQBand<FloatType, FilterChoices...>::setQValue (FloatType newQValue)
{
    qHandle = newQValue;
}

template <typename FloatType, typename... FilterChoices>
void EQBand<FloatType, FilterChoices...>::setGain (FloatType newGain)
{
    gainHandle = newGain;
}

template <typename FloatType, typename... FilterChoices>
void EQBand<FloatType, FilterChoices...>::setGainDB (FloatType newGainDB)
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
    fs = (FloatType) spec.sampleRate;

    fadeBuffer.setSize ((int) spec.numChannels, (int) spec.maximumBlockSize);
    fadeBuffer.clear();

    eqband_detail::forEachInTuple ([spec] (auto& filter, size_t) { filter.prepare ((int) spec.numChannels); },
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
    eqband_detail::forEachInTuple ([] (auto& filter, size_t) { filter.reset(); },
                                   filters);

    for (auto* smoother : { &freqSmooth, &qSmooth, &gainSmooth })
        smoother->reset();

    prevFilterType = filterType;
}

template <typename FloatType, typename... FilterChoices>
template <typename FilterType>
void EQBand<FloatType, FilterChoices...>::processFilterChannel (FilterType& filter, juce::dsp::AudioBlock<FloatType>& block)
{
    auto setParams = [&filter, fs = this->fs] (FloatType curFreq, FloatType curQ, FloatType curGain) {
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
             gainValues = gainSmooth.getSmoothedBuffer()] (int n) { setParamsFunc (freqHzValues[n], qValues[n], gainValues[n]); });
    }
    else
    {
        setParams (freqSmooth.getCurrentValue(), qSmooth.getCurrentValue(), gainSmooth.getCurrentValue());
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
template <typename ProcessContext>
void EQBand<FloatType, FilterChoices...>::process (const ProcessContext& context)
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
    AudioBlockHelpers::copyBlocks (block, inputBlock);

    if (context.isBypassed)
    {
        reset();
        return;
    }

    const auto needsFade = filterType != prevFilterType;
    if (needsFade)
    {
        for (size_t channel = 0; channel < numChannels; ++channel)
            fadeBuffer.copyFrom ((int) channel, 0, block.getChannelPointer (channel), numSamples);
    }

    eqband_detail::forEachInTuple (
        [this, &block] (auto& filter, size_t filterIndex) {
            if ((int) filterIndex == filterType)
            {
                processFilterChannel (filter, block);
            }
            else if ((int) filterIndex == prevFilterType)
            {
                auto&& fadeBlock = juce::dsp::AudioBlock<FloatType> { fadeBuffer };
                processFilterChannel (filter, fadeBlock);
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

} // namespace chowdsp
