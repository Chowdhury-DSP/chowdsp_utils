#pragma once

namespace chowdsp
{
/** An N-way crossover filter based using internal Linkwitz-Riley filters */
template <typename T, int Order, int NumBands>
class CrossoverFilter
{
    static_assert (NumBands > 2, "NumBands must be > 2!");

public:
    CrossoverFilter() = default;

    /** Prepares the filter to process a new stram of audio. */
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        lowerBandsCrossover.prepare (spec);
        highCutFilter.prepare (spec);
        for (auto& filter : apHighCutFilter)
            filter.prepare (spec);

        tempBuffer.setMaxSize ((int) spec.numChannels, (int) spec.maximumBlockSize);
    }

    /** Resets the filter state. */
    void reset()
    {
        lowerBandsCrossover.reset();
        highCutFilter.reset();
        for (auto& filter : apHighCutFilter)
            filter.reset();
    }

    /**
     * Sets the crossover frequency for a given crossover in Hz.
     *
     * The lowest frequency crossover will have index 0, and the
     * highest frequency crossover will have frequency NumBands - 2.
     */
    void setCrossoverFrequency (int crossoverIndex, T freqHz)
    {
        jassert (crossoverIndex <= NumBands - 2);

        if (crossoverIndex == NumBands - 2)
        {
            highCutFilter.setCrossoverFrequency (freqHz);
            for (auto& filter : apHighCutFilter)
                filter.setCrossoverFrequency (freqHz);
        }
        else
        {
            lowerBandsCrossover.setCrossoverFrequency (crossoverIndex, freqHz);
        }
    }

    /**
     * Processes all bands of the crossover filter. The output bands
     * should be provided in order from lowest band to highest. The
     * size of the output buffer span must be equal to NumBands.
     */
    [[maybe_unused]] void processBlock (const BufferView<const T>& bufferIn,
                                        nonstd::span<const BufferView<T>> buffersOut) noexcept
    {
        jassert ((int) buffersOut.size() == NumBands);

        tempBuffer.setCurrentSize (bufferIn.getNumChannels(), bufferIn.getNumSamples());

        if constexpr (Order == 1)
        {
            auto lowerBandBuffers = buffersOut.template first<(size_t) NumBands - 1>();
            lowerBandsCrossover.processBlock (bufferIn, lowerBandBuffers);
            BufferMath::copyBufferData (lowerBandBuffers.back(), tempBuffer); // Order-1 LR filter does not allow pointer aliasing, so we copy to a temp buffer here.
            highCutFilter.processBlock (tempBuffer, lowerBandBuffers.back(), buffersOut.back());
        }
        else
        {
            auto lowerBandBuffers = buffersOut.template first<(size_t) NumBands - 1>();
            lowerBandsCrossover.processBlock (bufferIn, lowerBandBuffers);
            highCutFilter.processBlock (lowerBandBuffers.back(), lowerBandBuffers.back(), buffersOut.back());

            // an allpass LR-filter with the same crossover as the high-cut frequency
            // this puts the low band back in-phase with the high- and mid-bands.
            for (auto [buffer, filter] : chowdsp::zip (buffersOut.template first<(size_t) NumBands - 2>(), apHighCutFilter))
            {
                filter.processBlock (buffer, buffer, tempBuffer);
                BufferMath::addBufferData (tempBuffer, buffer);
            }
        }
    }

    /**
     * Processes all bands of the crossover filter. The output bands
     * should be provided in order from lowest band to highest. The
     * size of the output buffer list must be equal to NumBands.
     */
    [[maybe_unused]] void processBlock (const BufferView<const T>& bufferIn,
                                        std::initializer_list<BufferView<T>>&& buffersOut) noexcept
    {
        processBlock (bufferIn, { buffersOut.begin(), buffersOut.end() });
    }

private:
    CrossoverFilter<T, Order, NumBands - 1> lowerBandsCrossover {};
    LinkwitzRileyFilter<T, Order> highCutFilter {};
    std::array<LinkwitzRileyFilter<T, Order>, (size_t) NumBands - 2> apHighCutFilter {};

    Buffer<T> tempBuffer {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CrossoverFilter)
};

/** An 2-way crossover filter based using an internal Linkwitz-Riley filter */
template <typename T, int Order>
class CrossoverFilter<T, Order, 2>
{
public:
    CrossoverFilter() = default;

    /** Prepares the filter to process a new stram of audio. */
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        filter.prepare (spec);
    }

    /** Resets the filter state. */
    void reset()
    {
        filter.reset();
    }

    /** Sets the crossover frequency in Hz. crossoverIndex must be equal to 0 */
    void setCrossoverFrequency ([[maybe_unused]] int crossoverIndex, T freqHz)
    {
        jassert (crossoverIndex == 0);
        filter.setCrossoverFrequency (freqHz);
    }

    /**
     * Processes all bands of the crossover filter. The output bands
     * should be provided in order from lowest band to highest. The
     * size of the output buffer span must be equal to 2.
     */
    [[maybe_unused]] void processBlock (const BufferView<const T>& bufferIn,
                                        nonstd::span<const BufferView<T>> buffersOut) noexcept
    {
        jassert ((int) buffersOut.size() == 2);
        filter.processBlock (bufferIn, buffersOut[0], buffersOut[1]);
    }

    /**
     * Processes all bands of the crossover filter. The output bands
     * should be provided in order from lowest band to highest. The
     * size of the output buffer list must be equal to 2.
     */
    [[maybe_unused]] void processBlock (const BufferView<const T>& bufferIn,
                                        std::initializer_list<BufferView<T>>&& buffersOut) noexcept
    {
        processBlock (bufferIn, { buffersOut.begin(), buffersOut.end() });
    }

private:
    LinkwitzRileyFilter<T, Order> filter {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CrossoverFilter)
};
} // namespace chowdsp
