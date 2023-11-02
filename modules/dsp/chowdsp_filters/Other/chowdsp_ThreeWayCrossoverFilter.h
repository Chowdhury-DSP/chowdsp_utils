#pragma once

namespace chowdsp
{
/** A 3-way crossover filter based using internal Linkwitz-Riley filters */
template <typename T, int Order>
class ThreeWayCrossoverFilter : public CrossoverFilter<T, Order, 3>
{
public:
    ThreeWayCrossoverFilter() = default;

    /** Sets the crossover frequency between the low- and mid-bands. */
    void setLowCrossoverFrequency (T freqHz)
    {
        this->setCrossoverFrequency (0, freqHz);
    }

    /** Sets the crossover frequency between the mid- and high-bands. */
    void setHighCrossoverFrequency (T freqHz)
    {
        this->setCrossoverFrequency (1, freqHz);
    }

    /** Processes the all bands of the crossover filter */
    [[maybe_unused]] void processBlock (const BufferView<const T>& bufferIn,
                                        const BufferView<T>& bufferLow,
                                        const BufferView<T>& bufferMid,
                                        const BufferView<T>& bufferHigh) noexcept
    {
        CrossoverFilter<T, Order, 3>::processBlock (bufferIn, { bufferLow, bufferMid, bufferHigh });
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThreeWayCrossoverFilter)
};
} // namespace chowdsp
