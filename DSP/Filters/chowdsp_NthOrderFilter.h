#pragma once

namespace chowdsp
{
/**
 * A Nth-order filter, either Lowpass, Highpass, or Bandpass,
 * composed of State Variable Filters with Butterworth Q, so
 * the filter can be modulated pretty fast.
 */
template <typename T, size_t order = 4, StateVariableFilterType type = StateVariableFilterType::Lowpass>
class NthOrderFilter : public juce::dsp::ProcessorBase
{
public:
    NthOrderFilter() : butterQVals (QValCalcs::butterworth_Qs<T, order>())
    {
        for (size_t i = 0; i < nFilters; ++i)
        {
            filters[i].setResonance (butterQVals[i]);
        }
    }

    /** Prepares the filter to process an audio stream */
    void prepare (const juce::dsp::ProcessSpec& spec) override
    {
        for (auto& filt : filters)
            filt.prepare (spec);
    }

    /** Resets the state of the filter */
    void reset() override
    {
        for (auto& filt : filters)
            filt.reset();
    }

    /** Sets the filter cutoff frequency in Hz */
    void setCutoffFrequency (float freq)
    {
        for (auto& filt : filters)
            filt.setCutoffFrequency (freq);
    }

    /** Sets the Q-value of the filter */
    void setQValue (float qVal)
    {
        filters[0].setResonance (butterQVals[0] * qVal * juce::MathConstants<T>::sqrt2);
    }

    /** Processes a block of samples */
    void process (const juce::dsp::ProcessContextReplacing<T>& context) override
    {
        for (auto& filt : filters)
            filt.template process<juce::dsp::ProcessContextReplacing<T>, type> (context);
    }

    /** Processes a single sample */
    inline T processSample (int channel, T x) noexcept
    {
        for (auto& filt : filters)
            x = filt.processSample (channel, x);
        return x;
    }

private:
    static constexpr size_t nFilters = order / 2;

    chowdsp::StateVariableFilter<T> filters[nFilters];
    const std::array<T, nFilters> butterQVals;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NthOrderFilter)
};

} // namespace chowdsp
