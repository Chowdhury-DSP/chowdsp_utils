#include "test_utils.h"
#include <JuceHeader.h>

namespace
{
constexpr float fs = 44100.0f;
}

/** Unit tests for chowdsp::Gain */
class GainTest : public UnitTest
{
public:
    GainTest() : UnitTest ("Gain Test") {}

    void gainTest (float gain)
    {
        chowdsp::GainProcessor gainProc;
        gainProc.setGain (gain);
        gainProc.reset();

        auto buffer = test_utils::makeNoise (fs, 0.25f);
        const int numSamples = buffer.getNumSamples();
        auto refMag = buffer.getMagnitude (0, numSamples);

        gainProc.processBlock (buffer);
        auto mag = buffer.getMagnitude (0, numSamples);

        expectWithinAbsoluteError (mag / refMag, gain, (float) 1.0e-6, "Incorrect gain!");
    }

    void runTest() override
    {
        beginTest ("Flat Test");
        gainTest (1.0f);

        beginTest ("Boost Test");
        gainTest (2.0f);

        beginTest ("Cut Test");
        gainTest (0.5f);
    }
};

static GainTest gainTest;
