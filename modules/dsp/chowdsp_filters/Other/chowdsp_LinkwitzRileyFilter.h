#pragma once

namespace chowdsp
{
/** Forward declaration for Liinkwitz-Riley filter classes */
template <typename T, int Order, typename Enable = void>
class LinkwitzRileyFilter;

/**
 * Linkwitz-Riley Crossover Filter
 *
 * Specialised for filters of order 2, 4, 8, 12, ...
 *
 * This filter will be stable under fast modulation.
 */
template <typename T, int Order>
class LinkwitzRileyFilter<T, Order, std::enable_if_t<(Order > 1) && (Order == 2 || (Order / 2) % 2 == 0)>>
{
    // for order 2 we need to flip the phase of the highpass output
    static constexpr auto needsPhaseFlip = Order == 2;

public:
    LinkwitzRileyFilter() = default;

    /** Prepares the filter to process a new stram of audio. */
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        crossoverFilter.prepare (spec);

        for (auto& filt : lpfs)
            filt.prepare (spec);

        for (auto& filt : hpfs)
            filt.prepare (spec);

        if constexpr (needsPhaseFlip)
        {
            crossoverFilter.setQValue ((T) 0.5);

            static_assert (numLeftoverFilters == halfFilterOrder);
            for (size_t i = 0; i < (size_t) numLeftoverFilters; ++i)
            {
                const auto qValIndex = i >= qVals.size() ? i - qVals.size() : i;
                lpfs[i].setQValue (qVals[qValIndex]);
                hpfs[i].setQValue (qVals[qValIndex]);
            }
        }
        else
        {
            crossoverFilter.setQValue (qVals[0]);

            static_assert (numLeftoverFilters + 1 == halfFilterOrder);
            for (size_t i = 0; i < (size_t) numLeftoverFilters; ++i)
            {
                const auto qValIndex = (i + 1) >= qVals.size() ? (i + 1) - qVals.size() : (i + 1);
                lpfs[i].setQValue (qVals[qValIndex]);
                hpfs[i].setQValue (qVals[qValIndex]);
            }
        }
    }

    /** Sets a new crossover frequency for the filter */
    void setCrossoverFrequency (T freqHz)
    {
        crossoverFilter.setCutoffFrequency (freqHz);

        for (auto& filt : lpfs)
            filt.setCutoffFrequency (freqHz);

        for (auto& filt : hpfs)
            filt.setCutoffFrequency (freqHz);
    }

    /** Reset's the filter's internal state */
    void reset()
    {
        crossoverFilter.reset();

        for (auto& filt : lpfs)
            filt.reset();

        for (auto& filt : hpfs)
            filt.reset();
    }

    /** Processes the lowpass part of the crossover filter */
    [[maybe_unused]] void processBlock (const BufferView<const T>& bufferIn,
                                        const BufferView<T>& bufferLow,
                                        const BufferView<T>& bufferHigh) noexcept
    {
        crossoverFilter.processBlock (bufferIn, bufferLow, bufferHigh);

        for (auto& filt : lpfs)
            filt.processBlock (bufferLow);

        for (auto& filt : hpfs)
            filt.processBlock (bufferHigh);

        if constexpr (! needsPhaseFlip)
        {
            // the crossover filter does a phase flip by default, so we have to undo it
            for (int ch = 0; ch < bufferHigh.getNumChannels(); ++ch)
                juce::FloatVectorOperations::negate (bufferHigh.getWritePointer (ch),
                                                     bufferHigh.getReadPointer (ch),
                                                     bufferHigh.getNumSamples());
        }
    }

    /** Processes a single-sample through the crossover filter */
    [[maybe_unused]] inline auto processSample (int channel, T x) noexcept
    {
        auto y = crossoverFilter.processSample (channel, x);

        for (auto& filt : lpfs)
            filt.processSample (channel, y.first);

        for (auto& filt : hpfs)
            filt.processSample (channel, y.second);

        // the crossover filter does a phase flip by default, so we have to undo it
        if constexpr (! needsPhaseFlip)
            y.second = -y.second;

        return y;
    }

private:
    StateVariableFilter<T, StateVariableFilterType::Crossover> crossoverFilter;

    static constexpr auto numLeftoverFilters = (Order - 2) / 2;
    std::array<StateVariableFilter<T, StateVariableFilterType::Lowpass>, (size_t) numLeftoverFilters> lpfs;
    std::array<StateVariableFilter<T, StateVariableFilterType::Highpass>, (size_t) numLeftoverFilters> hpfs;

    static constexpr auto halfFilterOrder = needsPhaseFlip ? numLeftoverFilters : (Order / 2);
    static constexpr auto qVals = QValCalcs::butterworth_Qs<SampleTypeHelpers::NumericType<T>, halfFilterOrder>();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LinkwitzRileyFilter)
};

#if CHOWDSP_USING_JUCE
/**
 * Linkwitz-Riley Crossover Filter
 *
 * Specialised for first-order filters.
 *
 * This filter will be stable under fast modulation.
 */
template <typename T, int Order>
class LinkwitzRileyFilter<T, Order, std::enable_if_t<Order == 1>>
{
    // for order 2 we need to flip the phase of the highpass output
    static constexpr auto needsPhaseFlip = Order == 2;

public:
    LinkwitzRileyFilter() = default;

    /** Prepares the filter to process a new stram of audio. */
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        fs = (SampleTypeHelpers::NumericType<T>) spec.sampleRate;
        lowFilter.prepare (spec);
        highFilter.prepare (spec);
    }

    /** Sets a new crossover frequency for the filter */
    void setCrossoverFrequency (T freqHz)
    {
        lowFilter.calcCoefs (freqHz, fs);
        highFilter.calcCoefs (freqHz, fs);
    }

    /** Reset's the filter's internal state */
    void reset()
    {
        lowFilter.reset();
        highFilter.reset();
    }

    /** Processes the both bands of the crossover filter */
    [[maybe_unused]] void processBlock (const BufferView<const T>& bufferIn,
                                        const BufferView<T>& bufferLow,
                                        const BufferView<T>& bufferHigh) noexcept
    {
        // no pointer aliasing!!
        jassert (bufferIn.getReadPointer (0) != bufferLow.getReadPointer (0));
        jassert (bufferIn.getReadPointer (0) != bufferHigh.getReadPointer (0));

        const auto&& inBlock = bufferIn.toAudioBlock();
        auto&& lowBlock = bufferLow.toAudioBlock();
        auto&& highBlock = bufferHigh.toAudioBlock();

        lowFilter.process (juce::dsp::ProcessContextNonReplacing<T> { inBlock, lowBlock });
        highFilter.process (juce::dsp::ProcessContextNonReplacing<T> { inBlock, highBlock });
    }

private:
    ModFilterWrapper<FirstOrderLPF<float>> lowFilter;
    ModFilterWrapper<FirstOrderHPF<float>> highFilter;

    SampleTypeHelpers::NumericType<T> fs {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LinkwitzRileyFilter)
};
#endif // CHOWDSP_USING_JUCE
} // namespace chowdsp
