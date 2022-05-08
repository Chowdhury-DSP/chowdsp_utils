#include <TimedUnitTest.h>
#include <chowdsp_parameters/chowdsp_parameters.h>

class RhythmParameterTest : public TimedUnitTest
{
public:
    RhythmParameterTest() : TimedUnitTest ("Rhythm Parameter Test", "Parameters") {}

    void runTestTimed() override
    {
        beginTest ("Rhythm Parameter Test");
        auto&& param = chowdsp::RhythmParameter ("rhythm", "Rhythm");
        expectEquals (param.getCurrentChoiceName(), juce::String ("1/4"), "Parameter choice label is incorrect!");
        expectEquals (param.getRhythmTimeSeconds (60.0), 1.0, "Quarter Note rhythm time is incorrect!");
    }
};

static RhythmParameterTest rhythmParameterTest;
