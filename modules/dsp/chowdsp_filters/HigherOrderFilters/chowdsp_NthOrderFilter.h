#pragma once

namespace chowdsp
{
/**
 * A Nth-order filter, either Lowpass, Highpass, or Bandpass,
 * composed of State Variable Filters with Butterworth Q, so
 * the filter can be modulated pretty fast.
 */
template <typename T, size_t order = 4, StateVariableFilterType type = StateVariableFilterType::Lowpass>
class NthOrderFilter
{
public:
    using NumericType = SampleTypeHelpers::NumericType<T>;
    static constexpr auto Order = order;
    static constexpr auto Type = type;

    static_assert (type == StateVariableFilterType::Lowpass
                       || type == StateVariableFilterType::Highpass
                       || type == StateVariableFilterType::Bandpass,
                   "NthOrderFilter is not defined for this filter type!");

    NthOrderFilter() : butterQVals (QValCalcs::butterworth_Qs<NumericType, order>())
    {
        for (size_t i = 0; i < nFilters; ++i)
            filters[i].setQValue (butterQVals[i]);
    }

    /** Prepares the filter to process an audio stream */
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        for (auto& filt : filters)
            filt.prepare (spec);
    }

    /** Resets the state of the filter */
    void reset()
    {
        for (auto& filt : filters)
            filt.reset();
    }

    /** Sets the filter cutoff frequency in Hz */
    void setCutoffFrequency (T freq)
    {
        for (auto& filt : filters)
            filt.setCutoffFrequency (freq);
    }

    /** Sets the Q-value of the filter */
    void setQValue (T qVal)
    {
        filters[0].setQValue (butterQVals[0] * qVal * juce::MathConstants<NumericType>::sqrt2);
    }

    /** Processes a block of samples */
    void processBlock (const chowdsp::BufferView<T>& buffer)
    {
        for (auto& filt : filters)
            filt.processBlock (buffer);
    }

#if JUCE_MODULE_AVAILABLE_juce_dsp
    /** Processes a block of samples */
    void process (const chowdsp::ProcessContextReplacing<T>& context)
    {
        for (auto& filt : filters)
            filt.process (context);
    }
#endif

    /** Processes a single sample */
    inline T processSample (int channel, T x) noexcept
    {
        for (auto& filt : filters)
            x = filt.processSample (channel, x);
        return x;
    }

private:
    static constexpr size_t nFilters = order / 2;

    chowdsp::StateVariableFilter<T, type> filters[nFilters];
    const std::array<NumericType, nFilters> butterQVals;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NthOrderFilter)
};

} // namespace chowdsp
