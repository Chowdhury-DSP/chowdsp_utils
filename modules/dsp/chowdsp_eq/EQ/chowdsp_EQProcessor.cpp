namespace chowdsp::EQ
{
template <typename FloatType, size_t numBands, typename EQBandType>
EQProcessor<FloatType, numBands, EQBandType>::EQProcessor() = default;

template <typename FloatType, size_t numBands, typename EQBandType>
void EQProcessor<FloatType, numBands, EQBandType>::setCutoffFrequency (int band, NumericType newCutoffHz)
{
    jassert (juce::isPositiveAndBelow (band, (int) numBands));
    bands[(size_t) band].setCutoffFrequency (newCutoffHz);
}

template <typename FloatType, size_t numBands, typename EQBandType>
void EQProcessor<FloatType, numBands, EQBandType>::setQValue (int band, NumericType newQValue)
{
    jassert (juce::isPositiveAndBelow (band, (int) numBands));
    bands[(size_t) band].setQValue (newQValue);
}

template <typename FloatType, size_t numBands, typename EQBandType>
void EQProcessor<FloatType, numBands, EQBandType>::setGain (int band, NumericType newGain)
{
    jassert (juce::isPositiveAndBelow (band, (int) numBands));
    bands[(size_t) band].setGain (newGain);
}

template <typename FloatType, size_t numBands, typename EQBandType>
void EQProcessor<FloatType, numBands, EQBandType>::setGainDB (int band, NumericType newGainDB)
{
    jassert (juce::isPositiveAndBelow (band, (int) numBands));
    bands[(size_t) band].setGainDB (newGainDB);
}

template <typename FloatType, size_t numBands, typename EQBandType>
void EQProcessor<FloatType, numBands, EQBandType>::setFilterType (int band, int newFilterType)
{
    jassert (juce::isPositiveAndBelow (band, (int) numBands));
    bands[(size_t) band].setFilterType (newFilterType);
}

template <typename FloatType, size_t numBands, typename EQBandType>
void EQProcessor<FloatType, numBands, EQBandType>::setBandOnOff (int band, bool shouldBeOn)
{
    jassert (juce::isPositiveAndBelow (band, (int) numBands));
    onOffs[(size_t) band] = shouldBeOn;
}

template <typename FloatType, size_t numBands, typename EQBandType>
void EQProcessor<FloatType, numBands, EQBandType>::prepare (const juce::dsp::ProcessSpec& spec)
{
    for (size_t i = 0; i < numBands; ++i)
    {
        bands[i].prepare (spec);
        bypasses[i].prepare (spec, onOffs[i]);
    }
}

template <typename FloatType, size_t numBands, typename EQBandType>
void EQProcessor<FloatType, numBands, EQBandType>::reset()
{
    for (auto& band : bands)
        band.reset();
}

template <typename FloatType, size_t numBands, typename EQBandType>
void EQProcessor<FloatType, numBands, EQBandType>::processBlock (const BufferView<FloatType>& block) noexcept
{
    for (size_t i = 0; i < numBands; ++i)
    {
        if (! bypasses[i].processBlockIn (block, onOffs[i]))
            continue;

        bands[i].processBlock (block);

        bypasses[i].processBlockOut (block, onOffs[i]);
    }
}
} // namespace chowdsp::EQ
