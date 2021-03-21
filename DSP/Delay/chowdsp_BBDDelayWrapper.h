#pragma once

namespace chowdsp
{

template <size_t STAGES>
class BBDDelayWrapper : public DelayLineBase<float>
{
public:
    BBDDelayWrapper() = default;

    void setDelay (float newDelayInSamples) override
    {
        delaySamp = newDelayInSamples;
        for (auto& line : lines)
            line.setDelay (delaySamp / sampleRate);
    }

    float getDelay() const override { return delaySamp; }

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

    void reset() override
    {
        for (auto& line : lines)
            line.reset();
    }

    inline void pushSample (int channel, float sample) noexcept override
    {
        inputs[channel] = sample;
    }

    inline float popSample (int channel) noexcept override
    {
        return lines[channel].process (inputs[channel]);
    }

    inline float popSample (int channel, float delayInSamples, bool /*updateReadPointer*/) noexcept override
    {
        setDelay (delayInSamples);
        return lines[channel].process (inputs[channel]);
    }

private:
    float delaySamp;
    float sampleRate;

    std::vector<BBDDelayLine<STAGES>> lines;
    std::vector<float> inputs;
};

} // namespace chowdsp
