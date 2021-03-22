#pragma once

namespace chowdsp
{
/** A dummy synthesiser sound that applies to all notes and channels */
struct DummySound : public juce::SynthesiserSound
{
    DummySound() = default;

    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

} // namespace chowdsp
