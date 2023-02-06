#include "PrototypeEQ.h"

void PrototypeEQ::setParameters (const Params& params)
{
    EQParams::setEQParameters (eq, params);
}

void PrototypeEQ::prepare (const juce::dsp::ProcessSpec& spec)
{
    eq.prepare (spec);
}

void PrototypeEQ::reset()
{
    eq.reset();
}

void PrototypeEQ::processBlock (const chowdsp::BufferView<FloatType>& buffer)
{
    eq.processBlock (buffer);
}
