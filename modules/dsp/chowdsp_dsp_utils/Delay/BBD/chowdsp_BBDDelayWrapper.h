#pragma once

namespace chowdsp::BBD
{
/**
 * A class that wraps the BBDDelayLine class
 * as a chowdsp::DelayLineBase<float>
 * 
 * @attention make sure to call `setFilterFreq()` every block
 *            of samples (or every few blocks) to make sure the
 *            BBD internal clock is correctly aligned.           
 */
template <size_t STAGES, bool ALIEN = false>
class BBDDelayWrapper : public DelayLineBase<float>
{
public:
    BBDDelayWrapper() = default;

    /** Sets the anti-aliasing filter frequency */
    void setFilterFreq (float freqHz)
    {
        setInputFilterFreq (freqHz);
        setOutputFilterFreq (freqHz);
    }

    /** Sets the input anti-imaging filter frequency */
    void setInputFilterFreq (float freqHz)
    {
        for (auto& line : lines)
            line.setInputFilterFreq (freqHz);
    }

    /** Sets the output anti-aliasing filter frequency */
    void setOutputFilterFreq (float freqHz)
    {
        for (auto& line : lines)
            line.setOutputFilterFreq (freqHz);
    }

    /** Sets the delay length in samples */
    void setDelay (float newDelayInSamples) final
    {
        delaySamp = newDelayInSamples;
        auto delaySec = delaySamp / sampleRate;
        for (auto& line : lines)
            line.setDelayTime (delaySec);
    }

    /** Returns the delay length in samples */
    [[nodiscard]] float getDelay() const final { return delaySamp; }

    /** Prepares the delay line for processing */
    void prepare (const juce::dsp::ProcessSpec& spec) final
    {
        sampleRate = (float) spec.sampleRate;
        inputs.resize (spec.numChannels, 0.0f);

        lines.clear();
        for (size_t ch = 0; ch < spec.numChannels; ++ch)
        {
            lines.emplace_back();
            lines[ch].prepare (sampleRate);
            lines[ch].setInputFilterFreq();
            lines[ch].setOutputFilterFreq();
        }
    }

    /** Frees internal memory */
    void free() final
    {
        inputs.clear();
        lines.clear();
    }

    /** Resets the state of the delay line */
    void reset() final
    {
        for (auto& line : lines)
            line.reset();
    }

    /** Pushes a sample into the delay line */
    inline void pushSample (int channel, float sample) noexcept final
    {
        inputs[(size_t) channel] = sample;
    }

    /** Returns a sample from the delay line */
    inline float popSample (int channel) noexcept final
    {
        return lines[(size_t) channel].process (inputs[(size_t) channel]);
    }

private:
    /** Returns a sample from the delay line. Note that the read pointer is always updated. */
    inline float popSample (int /*channel*/, float /*delayInSamples*/, bool /*updateReadPointer*/) noexcept final
    {
        return 0.0f;
    }

    /** Increment the read pointer without reading an interpolated sample (be careful...) */
    inline void incrementReadPointer (int /*channel*/) noexcept final
    {
    }

    float delaySamp = 1.0f;
    float sampleRate = 48000.0f;

    std::vector<BBDDelayLine<STAGES, ALIEN>> lines;
    std::vector<float> inputs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BBDDelayWrapper)
};

} // namespace chowdsp::BBD
