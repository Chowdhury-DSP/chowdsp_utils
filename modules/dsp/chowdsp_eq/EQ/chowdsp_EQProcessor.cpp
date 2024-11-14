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
void EQProcessor<FloatType, numBands, EQBandType>::prepare (const juce::dsp::ProcessSpec& spec, bool useInternalArena)
{
    for (size_t i = 0; i < numBands; ++i)
    {
        bands[i].prepare (spec);
        bypasses[i].prepare (spec, onOffs[i], false);
    }

    const auto paddedChannelSize = Math::round_to_next_multiple (static_cast<size_t> (spec.maximumBlockSize), SIMDUtils::defaultSIMDAlignment);
    requiredMemoryBytes = paddedChannelSize * sizeof (FloatType) * 3 // per-band smoothed values
                          + paddedChannelSize * spec.numChannels * sizeof (FloatType) * 2; // per-band fade and bypass buffers

    if (useInternalArena)
        internalArena.reset (requiredMemoryBytes + 32);
}

template <typename FloatType, size_t numBands, typename EQBandType>
void EQProcessor<FloatType, numBands, EQBandType>::reset()
{
    for (auto& band : bands)
        band.reset();
    for (auto [bypass, bandOnOff] : zip (bypasses, onOffs))
        bypass.reset (bandOnOff);
}

template <typename FloatType, size_t numBands, typename EQBandType>
void EQProcessor<FloatType, numBands, EQBandType>::processBlock (const BufferView<FloatType>& block) noexcept
{
    processBlock (block, internalArena);
}

template <typename FloatType, size_t numBands, typename EQBandType>
void EQProcessor<FloatType, numBands, EQBandType>::processBlock (const BufferView<FloatType>& block, ArenaAllocatorView arena) noexcept
{
    for (size_t i = 0; i < numBands; ++i)
    {
        const auto frame = arena.create_frame();
        if (! bypasses[i].processBlockIn (block, onOffs[i], arena))
        {
            bands[i].reset();
            continue;
        }

        bands[i].processBlock (block, arena);

        bypasses[i].processBlockOut (block, onOffs[i]);
    }
}
} // namespace chowdsp::EQ
