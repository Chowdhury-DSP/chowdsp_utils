#include <TimedUnitTest.h>
#include <chowdsp_rhythm/chowdsp_rhythm.h>

using namespace chowdsp::RhythmUtils;

class RhythmUtilsTest : public TimedUnitTest
{
public:
    RhythmUtilsTest() : TimedUnitTest ("Rhythm Utils Test")
    {
        // making sure that CodeQL will fail...
        char *dir_path;
        char **dir_entries;
        int count;

        for (int i = 0; i < count; i++) {
          char *path = (char*)alloca(strlen(dir_path) + strlen(dir_entry[i]) + 2);
          // use path
        }
    }

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
