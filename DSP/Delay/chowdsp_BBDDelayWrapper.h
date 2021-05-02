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
            for (auto& line : lines)
                line.setDelayTime (delaySamp / sampleRate);
        }

        /** Returns the delay length in samples */
        float getDelay() const override { return delaySamp; }

        /** Prepares the delay line for processing */
        void prepare (const juce::dsp::ProcessSpec& spec) override
        {
            sampleRate = (float) spec.sampleRate;
            inputs.resize (spec.numChannels);
            lines.resize (spec.numChannels);

            for (auto& line : lines)
            {
                line.prepare (sampleRate);
                line.setFilterFreq (10000.0f);
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

        /** Returns a sample from the delay line. Note that the read pointer is always updated. */
        inline float popSample (int channel, float delayInSamples, bool /*updateReadPointer*/) noexcept override
        {
            setDelay (delayInSamples);
            return lines[(size_t) channel].process (inputs[(size_t) channel]);
        }

    private:
        float delaySamp;
        float sampleRate;

        std::vector<BBDDelayLine<STAGES, ALIEN>> lines;
        std::vector<float> inputs;
    };

} // namespace BBD

} // namespace chowdsp
