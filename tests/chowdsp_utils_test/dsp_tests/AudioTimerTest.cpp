#include <TimedUnitTest.h>

class AudioTimerTest : public TimedUnitTest
{
public:
    AudioTimerTest() : TimedUnitTest ("Audio Timer Test") {}

    void audioTimerTest (double sampleRate)
    {
        chowdsp::AudioTimer timer;
        timer.prepare (sampleRate);

        {
            timer.reset();
            timer.advance (1000);
            timer.advance (500);
            timer.advance (100);

            expectEquals (timer.getTimeSamples(), (int64) 1600, "Sample time is incorrect!");
            expectWithinAbsoluteError (timer.getTimeMilliseconds(), 1600.0 * (1000.0 / sampleRate), 1.0e-12, "Milliseconds time is incorrect");
            expectWithinAbsoluteError (timer.getTimeSeconds(), 1600.0 / sampleRate, 1.0e-12, "Seconds time is incorrect");
        }

        {
            timer.reset();
            timer.advance (2000);
            timer.advance (500);

            expectEquals (timer.getTimeSamples(), (int64) 2500, "Sample time is incorrect!");
            expectWithinAbsoluteError (timer.getTimeMilliseconds(), 2500.0 * (1000.0 / sampleRate), 1.0e-12, "Milliseconds time is incorrect");
            expectWithinAbsoluteError (timer.getTimeSeconds(), 2500.0 / sampleRate, 1.0e-12, "Seconds time is incorrect");
        }
    }

    void runTestTimed() override
    {
        beginTest ("Audio Timer Test");
        audioTimerTest (44100.0);
        audioTimerTest (48000.0);
        audioTimerTest (96000.0);
    }
};

static AudioTimerTest audioTimerTest;
