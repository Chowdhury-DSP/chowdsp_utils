#include <CatchUtils.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

static void audioTimerTest (double sampleRate)
{
    chowdsp::AudioTimer timer;
    timer.prepare (sampleRate);

    {
        timer.reset();
        timer.advance (1000);
        timer.advance (500);
        timer.advance (100);

        REQUIRE_MESSAGE (timer.getTimeSamples() == (juce::int64) 1600, "Sample time is incorrect!");
        REQUIRE_MESSAGE (timer.getTimeMilliseconds() == Catch::Approx (1600.0 * (1000.0 / sampleRate)).margin (1.0e-12), "Milliseconds time is incorrect");
        REQUIRE_MESSAGE (timer.getTimeSeconds() == Catch::Approx (1600.0 / sampleRate).margin (1.0e-12), "Seconds time is incorrect");
    }

    {
        timer.reset();
        timer.advance (2000);
        timer.advance (500);

        REQUIRE_MESSAGE (timer.getTimeSamples() == (juce::int64) 2500, "Sample time is incorrect!");
        REQUIRE_MESSAGE (timer.getTimeMilliseconds() == Catch::Approx (2500.0 * (1000.0 / sampleRate)).margin (1.0e-12), "Milliseconds time is incorrect");
        REQUIRE_MESSAGE (timer.getTimeSeconds() == Catch::Approx (2500.0 / sampleRate).margin (1.0e-12), "Seconds time is incorrect");
    }
}

TEST_CASE ("Audio Timer Test", "[dsp][misc]")
{
    audioTimerTest (44100.0);
    audioTimerTest (48000.0);
    audioTimerTest (96000.0);
}
