#include "PrototypeEQ.h"

void PrototypeEQ::setParameters (const Params& params)
{
    for (size_t i = 0; i < Params::numBands; ++i)
    {
        bands[i].setCutoffFrequency (params.bands[i].bandFreqHz);
        bands[i].setQValue (params.bands[i].bandQ);
        bands[i].setGainDB (params.bands[i].bandGainDB);
        bands[i].setFilterType (params.bands[i].bandType);
    }
}

void PrototypeEQ::prepare (const juce::dsp::ProcessSpec& spec)
{
    for (auto& band : bands)
        band.prepare (spec);
}

void PrototypeEQ::reset()
{
    for (auto& band : bands)
        band.reset();
}

void PrototypeEQ::processBlock (juce::AudioBuffer<float>& buffer)
{
    auto&& block = juce::dsp::AudioBlock<float> { buffer };
    auto&& context = juce::dsp::ProcessContextReplacing<float> { block };

    for (auto& band : bands)
        band.process (context);
}
