#include <test_utils.h>
#include <TimedUnitTest.h>

#include "BassmanToneStack.h"
#include "BaxandallEQ.h"

namespace
{
constexpr double _fs = 48000.0;
} // namespace

class RTypeTest : public TimedUnitTest
{
public:
    RTypeTest() : TimedUnitTest ("Wave Digital Filter R-Type Test", "Wave Digital Filters") {}

    void bassmanFreqTest (float lowPot, float highPot, float sineFreq, float expGainDB, float maxErr)
    {
        Tonestack tonestack;
        tonestack.prepare (_fs);
        tonestack.setParams ((double) highPot, (double) lowPot, 1.0);

        auto buffer = test_utils::makeSineWave (sineFreq, (float) _fs, 1.0f);
        auto* x = buffer.getWritePointer (0);
        for (int n = 0; n < buffer.getNumSamples(); ++n)
            x[n] = (float) tonestack.processSample ((double) x[n]);

        auto actualGainDB = Decibels::gainToDecibels (buffer.getMagnitude (1000, buffer.getNumSamples() - 1000));
        expectWithinAbsoluteError (actualGainDB, expGainDB, maxErr);
    }

    void baxandallFreqTest (float bassParam, float trebleParam, float sineFreq, float expGainDB, float maxErr)
    {
        BaxandallWDF baxandall;
        baxandall.prepare (_fs);
        baxandall.setParams (bassParam, trebleParam);

        auto buffer = test_utils::makeSineWave (sineFreq, (float) _fs, 1.0f);
        auto* x = buffer.getWritePointer (0);
        for (int n = 0; n < buffer.getNumSamples(); ++n)
            x[n] = baxandall.processSample (x[n]);

        auto actualGainDB = Decibels::gainToDecibels (buffer.getMagnitude (1000, buffer.getNumSamples() - 1000));
        expectWithinAbsoluteError (actualGainDB, expGainDB, maxErr);
    }

    void runTestTimed() override
    {
        beginTest ("Bassman Bass Test");
        bassmanFreqTest (0.5f, 0.001f, 60.0f, -9.0f, 0.5f);

        beginTest ("Bassman Treble Test");
        bassmanFreqTest (0.999f, 0.999f, 15000.0f, 5.0f, 0.5f);

        beginTest ("Baxandall Bass Test");
        baxandallFreqTest (0.0001f, 0.1f, 20.0f, -3.0f, 0.5f);

        beginTest ("Baxandall Treble Test");
        baxandallFreqTest (0.1f, 0.0001f, 20000.0f, -8.0f, 0.5f);
    }
};

static RTypeTest rTypeTest;
