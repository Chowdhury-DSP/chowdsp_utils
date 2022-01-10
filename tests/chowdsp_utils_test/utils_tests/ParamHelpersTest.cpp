#include <TimedUnitTest.h>

using namespace chowdsp::ParamUtils;

class ParamHelpersTest : public TimedUnitTest
{
public:
    ParamHelpersTest() : TimedUnitTest ("Param Helpers Test") {}

    void checkRange (NormalisableRange<float> range, float start, float end, float centre)
    {
        constexpr float maxErr = 1.0e-3f;

        expectWithinAbsoluteError (range.convertFrom0to1 (0.0f), start, maxErr, "Converting from 0 -> 1 at start is incorrect!");
        expectWithinAbsoluteError (range.convertFrom0to1 (0.5f), centre, maxErr, "Converting from 0 -> 1 at centre is incorrect!");
        expectWithinAbsoluteError (range.convertFrom0to1 (1.0f), end, maxErr, "Converting from 0 -> 1 at end is incorrect!");

        expectWithinAbsoluteError (range.convertTo0to1 (start), 0.0f, maxErr, "Converting to 0 -> 1 at start is incorrect!");
        expectWithinAbsoluteError (range.convertTo0to1 (centre), 0.5f, maxErr, "Converting to 0 -> 1 at centre is incorrect!");
        expectWithinAbsoluteError (range.convertTo0to1 (end), 1.0f, maxErr, "Converting to 0 -> 1 at end is incorrect!");
    }

    void createNormRangeTest()
    {
        constexpr float start = 0.05f;
        constexpr float end = 1.5f;
        constexpr float centre = 1.35f;

        auto testRange = createNormalisableRange (start, end, centre);
        checkRange (testRange, start, end, centre);
    }

    void freqParamTest()
    {
        constexpr float start = 200.0f;
        constexpr float end = 20000.0f;
        constexpr float centre = 2000.0f;

        chowdsp::Parameters params;
        createFreqParameter (params, "test", "Test", start, end, centre, centre);
        checkRange (params[0]->getNormalisableRange(), start, end, centre);

        expectEquals (params[0]->getText (0.5f, 1024), String ("2.00 kHz"), "kHz value string is incorrect!");
        expectEquals (params[0]->getText (0.0f, 1024), String ("200.00 Hz"), "Hz value string is incorrect!");
    }

    void percentParamTest()
    {
        chowdsp::Parameters params;
        createPercentParameter (params, "test", "Test", 0.5f);

        expectEquals (params[0]->getText (0.5f, 1024), String ("50%"), "50% value string is incorrect!");
        expectEquals (params[0]->getText (0.0f, 1024), String ("0%"), "0% value string is incorrect!");
    }

    void gainDBParamTest()
    {
        chowdsp::Parameters params;
        createGainDBParameter (params, "test", "Test", -100.0f, 0.0f, -12.0f, -12.0f);

        expectEquals (params[0]->getText (0.5f, 1024), String ("-12.00 dB"), "-12 dB value string is incorrect!");
        expectEquals (params[0]->getText (1.0f, 1024), String ("0.00 dB"), "0 dB value string is incorrect!");
    }

    void runTestTimed() override
    {
        beginTest ("Create Normalisable Range Test");
        createNormRangeTest();

        beginTest ("Create Freq. Param Test");
        freqParamTest();

        beginTest ("Create Percent Param Test");
        percentParamTest();

        beginTest ("Create Gain DB Param Test");
        gainDBParamTest();
    }
};

static ParamHelpersTest paramHelpersTest;
