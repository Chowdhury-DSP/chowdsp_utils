#include <TimedUnitTest.h>
#include <chowdsp_rhythm/chowdsp_rhythm.h>

using namespace chowdsp::RhythmUtils;

class RhythmUtilsTest : public TimedUnitTest
{
public:
    RhythmUtilsTest() : TimedUnitTest ("Rhythm Utils Test") {}

    void runTestTimed() override
    {
        beginTest ("Rhythm Time Test");
        expectEquals (QUARTER.getTimeSeconds (60.0), 1.0, "Quarter Note time is incorrect!");

        beginTest ("Label Test");
        expectEquals (QUARTER.getLabel(), juce::String ("1/4"), "Quarter Note label is incorrect!");

        beginTest ("Equality Test");
        expect (QUARTER == QUARTER, "Quarter Notes are not equal!");
    }
};

class RhythmUtilsTest rhythmUtilsTest;
