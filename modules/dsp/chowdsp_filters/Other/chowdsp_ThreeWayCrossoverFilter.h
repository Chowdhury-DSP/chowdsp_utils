#pragma once

namespace chowdsp
{
/** A 3-way crossover filter based using internal Linkwitz-Riley filters */
template <typename T, int Order>
class ThreeWayCrossoverFilter
{
public:
    ThreeWayCrossoverFilter() = default;

    /** Prepares the filter to process a new stram of audio. */
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        lowCutFilter.prepare (spec);
        highCutFilter.prepare (spec);
        apHighCutFilter.prepare (spec);

        tempBuffer.setMaxSize ((int) spec.numChannels, (int) spec.maximumBlockSize);
    }

    /** Sets the crossover frequency between the low- and mid-bands. */
    void setLowCrossoverFrequency (T freqHz)
    {
        lowCutFilter.setCrossoverFrequency (freqHz);
    }

    /** Sets the crossover frequency between the mid- and high-bands. */
    void setHighCrossoverFrequency (T freqHz)
    {
        highCutFilter.setCrossoverFrequency (freqHz);
        apHighCutFilter.setCrossoverFrequency (freqHz);
    }

    /** Resets the filter state. */
    void reset()
    {
        lowCutFilter.reset();
        highCutFilter.reset();
        apHighCutFilter.reset();
    }

    /** Processes the all bands of the crossover filter */
    [[maybe_unused]] void processBlock (const BufferView<const T>& bufferIn,
                                        const BufferView<T>& bufferLow,
                                        const BufferView<T>& bufferMid,
                                        const BufferView<T>& bufferHigh) noexcept
    {
        tempBuffer.setCurrentSize (bufferIn.getNumChannels(), bufferIn.getNumSamples());

        if constexpr (Order == 1)
        {
            lowCutFilter.processBlock (bufferIn, bufferLow, bufferMid);
            BufferMath::copyBufferData (bufferMid, tempBuffer); // Order-1 LR filter does not allow pointer aliasing, so we copy to a temp buffer here.
            highCutFilter.processBlock (tempBuffer, bufferMid, bufferHigh);
        }
        else
        {
            lowCutFilter.processBlock (bufferIn, bufferLow, bufferMid);
            highCutFilter.processBlock (bufferMid, bufferMid, bufferHigh);

            // an allpass LR-filter with the same crossover as the high-cut frequency
            // this puts the low band back in-phase with the high- and mid-bands.
            apHighCutFilter.processBlock (bufferLow, bufferLow, tempBuffer);
            BufferMath::addBufferData (tempBuffer, bufferLow);
        }
    }

private:
    LinkwitzRileyFilter<T, Order> lowCutFilter, highCutFilter, apHighCutFilter;
    Buffer<T> tempBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThreeWayCrossoverFilter)
};
} // namespace chowdsp
