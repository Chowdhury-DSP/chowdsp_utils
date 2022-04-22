#include "PrototypeEQ.h"

void PrototypeEQ::setParameters (const Params& params)
{
    for (size_t i = 0; i < Params::numBands; ++i)
    {
        eq.setCutoffFrequency ((int) i, params.bands[i].bandFreqHz);
        eq.setQValue ((int) i, params.bands[i].bandQ);
        eq.setGainDB ((int) i, params.bands[i].bandGainDB);
        eq.setFilterType ((int) i, params.bands[i].bandType);
        eq.setBandOnOff ((int) i, params.bands[i].bandOnOff);
    }
}

void PrototypeEQ::prepare (const juce::dsp::ProcessSpec& spec)
{
    eq.prepare (spec);
}

void PrototypeEQ::reset()
{
    eq.reset();
}

void PrototypeEQ::processBlock (juce::AudioBuffer<float>& buffer)
{
    auto&& block = juce::dsp::AudioBlock<float> { buffer };
    eq.process (block);
}
