#pragma once

namespace chowdsp
{

template <typename FloatType, size_t numBands, typename EQBandType>
class EQProcessor
{
public:
    EQProcessor();

    void setCutoffFrequency (int band, FloatType newCutoffHz);
    void setQValue (int band, FloatType newQValue);
    void setGain (int band, FloatType newGain);
    void setGainDB (int band, FloatType newGainDB);
    void setFilterType (int band, int newFilterType);
    void setBandOnOff (int band, bool shouldBeOn);

    void prepare (const juce::dsp::ProcessSpec& spec);

    void reset();

    void process (juce::dsp::AudioBlock<FloatType> block);

private:
    std::array<EQBandType, numBands> bands;
    std::array<BypassProcessor<FloatType>, numBands> bypasses;
    std::array<bool, numBands> onOffs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQProcessor)
};
} // namespace chowdsp

#include "chowdsp_EQProcessor.cpp"
