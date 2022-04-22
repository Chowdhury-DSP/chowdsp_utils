#pragma once

#include <tuple>

namespace chowdsp
{
template <typename FloatType, typename... FilterChoices>
class EQBand
{
public:
    EQBand();

    void setCutoffFrequency (FloatType newCutoffHz);
    void setQValue (FloatType newQValue);
    void setGain (FloatType newGain);
    void setGainDB (FloatType newGainDB);
    void setFilterType (int newFilterType);

    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();

    template <typename ProcessContext>
    void process (const ProcessContext& context);

private:
    template <typename FilterType>
    void processFilterChannel (FilterType& filter, FloatType* samples, int numSamples);

    void fadeBuffers (const FloatType* fadeInBuffer, const FloatType* fadeOutBuffer, FloatType* targetBuffer, int numSamples);

    static constexpr auto numFilterChoices = sizeof...(FilterChoices);
    using Filters = std::tuple<FilterChoices...>;
    std::vector<Filters> filters;

    FloatType freqHzHandle = 1000.0f;
    FloatType qHandle = 0.7071f;
    FloatType gainHandle = 1.0f;

    SmoothedBufferValue<FloatType, juce::ValueSmoothingTypes::Multiplicative> freqSmooth;
    SmoothedBufferValue<FloatType, juce::ValueSmoothingTypes::Multiplicative> qSmooth;
    SmoothedBufferValue<FloatType, juce::ValueSmoothingTypes::Multiplicative> gainSmooth;
    int filterType = 0, prevFilterType = 0;

    FloatType fs = FloatType (44100.0);

    juce::AudioBuffer<FloatType> fadeBuffer;
};
} // namespace chowdsp

#include "chowdsp_EQBand.cpp"
