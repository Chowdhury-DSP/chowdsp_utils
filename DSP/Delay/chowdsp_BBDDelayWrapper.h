#pragma once

namespace chowdsp
{
namespace BBD
{
    /**
 * A class that wraps the BBDDelayLine class
 * as a chowdsp::DelayLineBase<float>
 */
    template <size_t STAGES, bool ALIEN = false>
    class BBDDelayWrapper : public DelayLineBase<float>
    {
    public:
        BBDDelayWrapper() = default;

        /** Sets the anti-aliasing filter frequency */
        void setFilterFreq (float freqHz)
        {
            for (auto& line : lines)
                line.setFilterFreq (freqHz);
        }

        /** Sets the delay length in samples */
        void setDelay (float newDelayInSamples) override
        {
            delaySamp = newDelayInSamples;
            auto delaySec = delaySamp / sampleRate;
            for (auto& line : lines)
                line.setDelayTime (delaySec);
        }

        /** Returns the delay length in samples */
        float getDelay() const override { return delaySamp; }

        /** Prepares the delay line for processing */
        void prepare (const juce::dsp::ProcessSpec& spec) override
        {
            sampleRate = (float) spec.sampleRate;
            inputs.resize (spec.numChannels, 0.0f);

            for (size_t ch = 0; ch < spec.numChannels; ++ch)
            {
                lines.emplace_back();
                lines[ch].prepare (sampleRate);
                lines[ch].setFilterFreq (10000.0f);
            }
        }

        /** Resets the state of the delay line */
        void reset() override
        {
            for (auto& line : lines)
                line.reset();
        }

        /** Pushes a sample into the delay line */
        inline void pushSample (int channel, float sample) noexcept override
        {
            inputs[(size_t) channel] = sample;
        }

        /** Returns a sample from the delay line */
        inline float popSample (int channel) noexcept override
        {
            return lines[(size_t) channel].process (inputs[(size_t) channel]);
        }

    private:
        /** Returns a sample from the delay line. Note that the read pointer is always updated. */
        inline float popSample (int /*channel*/, float /*delayInSamples*/, bool /*updateReadPointer*/) noexcept override
        {
            return 0.0f;
        }

        /** Increment the read pointer without reading an interpolated sample (be careful...) */
        inline void incrementReadPointer (int /*channel*/) noexcept override
        {
        }

        float delaySamp;
        float sampleRate;

        std::vector<BBDDelayLine<STAGES, ALIEN>> lines;
        std::vector<float> inputs;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BBDDelayWrapper)
    };

} // namespace BBD

} // namespace chowdsp
