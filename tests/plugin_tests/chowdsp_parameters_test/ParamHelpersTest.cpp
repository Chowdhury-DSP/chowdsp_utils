#include <TimedUnitTest.h>
#include <chowdsp_parameters/chowdsp_parameters.h>

using namespace chowdsp::ParamUtils;

class ParamHelpersTest : public TimedUnitTest
{
public:
    ParamHelpersTest() : TimedUnitTest ("Param Helpers Test", "Parameters") {}

    void checkRange (const juce::NormalisableRange<float>& range, float start, float end, float centre)
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

        expectEquals (params[0]->getText (0.5f, 1024), juce::String ("2.00 kHz"), "kHz value string is incorrect!");
        expectEquals (params[0]->getText (0.0f, 1024), juce::String ("200.00 Hz"), "Hz value string is incorrect!");
    }

    void percentParamTest()
    {
        chowdsp::Parameters params;
        createPercentParameter (params, "test", "Test", 0.5f);
        createBipolarPercentParameter (params, "test_bp", "Test");

        expectEquals (params[0]->getText (0.5f, 1024), juce::String ("50%"), "50% value string is incorrect!");
        expectEquals (params[0]->getText (0.0f, 1024), juce::String ("0%"), "0% value string is incorrect!");

        expectEquals (params[1]->getText (0.0f, 1024), juce::String ("-100%"), "-100% value string is incorrect!");
        expectEquals (params[1]->getText (0.5f, 1024), juce::String ("0%"), "0% value string is incorrect!");
        expectEquals (params[1]->getText (1.0f, 1024), juce::String ("100%"), "100% value string is incorrect!");
    }

    void gainDBParamTest()
    {
        chowdsp::Parameters params;
        createGainDBParameter (params, "test", "Test", -100.0f, 0.0f, -12.0f, -12.0f);

        expectEquals (params[0]->getText (0.5f, 1024), juce::String ("-12.00 dB"), "-12 dB value string is incorrect!");
        expectEquals (params[0]->getText (1.0f, 1024), juce::String ("0.00 dB"), "0 dB value string is incorrect!");
    }

    void timeMsParamTest()
    {
        chowdsp::Parameters params;
        createTimeMsParameter (params, "test", "Test", createNormalisableRange (0.0f, 2000.0f, 200.0f), 0.0f);

        expectEquals (params[0]->getText (0.0f, 1024), juce::String ("0.00 ms"), "0 ms value string is incorrect!");
        expectEquals (params[0]->getText (0.5f, 1024), juce::String ("200.00 ms"), "200 ms value string is incorrect!");
        expectEquals (params[0]->getText (1.0f, 1024), juce::String ("2.00 s"), "2 s value string is incorrect!");
    }

    void ratioParamTest()
    {
        chowdsp::Parameters params;
        createRatioParameter (params, "test", "Test", createNormalisableRange (0.5f, 5.0f, 1.0f), 1.0f);

        expectEquals (params[0]->getText (0.0f, 1024), juce::String ("0.50 : 1"), "0.5 : 1 value string is incorrect!");
        expectEquals (params[0]->getText (0.5f, 1024), juce::String ("1.00 : 1"), "1 : 1 value string is incorrect!");
        expectEquals (params[0]->getText (1.0f, 1024), juce::String ("5.00 : 1"), "5 : 1 value string is incorrect!");
    }

    void enumParamTest()
    {
        enum Mode
        {
            Mode_1,
            Mode_2,
            Mode_3,
        };

        chowdsp::EnumChoiceParameter<Mode> modeParam ("mode", "Mode", Mode::Mode_1);

        expect (modeParam.get() == Mode_1, "Default value is incorrect!");
        expectEquals (static_cast<juce::RangedAudioParameter&> (modeParam).getText (0.0f, 1024), juce::String ("Mode 1"), "Mode 1 value string is incorrect!");
        expectEquals (static_cast<juce::RangedAudioParameter&> (modeParam).getText (0.5f, 1024), juce::String ("Mode 2"), "Mode 2 value string is incorrect!");
        expectEquals (static_cast<juce::RangedAudioParameter&> (modeParam).getText (1.0f, 1024), juce::String ("Mode 3"), "Mode 3 value string is incorrect!");
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

        beginTest ("Create Time Milliseconds Param Test");
        timeMsParamTest();

        beginTest ("Create Ratio Param Test");
        ratioParamTest();

        beginTest ("Create Enum Param Test");
        enumParamTest();
    }
};

static ParamHelpersTest paramHelpersTest;
