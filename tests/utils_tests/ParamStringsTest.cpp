#include <JuceHeader.h>

using namespace chowdsp::ParamUtils;

class ParamStringsTest : public UnitTest
{
public:
    ParamStringsTest() : UnitTest ("Param Strings Test") {}

    void freqParamTest()
    {
        expectEquals (freqValToString (100.0f), String ("100.00 Hz"), "Incorrect small freq string!");
        expectEquals (freqValToString (10000.0f), String ("10.00 kHz"), "Incorrect large freq string!");
        expectEquals (stringToFreqVal ("100"), 100.0f, "Incorrect small freq value!");
        expectEquals (stringToFreqVal ("10k"), 10000.0f, "Incorrect large freq value!");
    }

    void percentParamTest()
    {
        expectEquals (percentValToString (0.5f), String ("50%"), "Incorrect percent string!");
        expectEquals (stringToPercentVal ("50%"), 0.5f, "Incorrect percent value!");
        expectEquals (stringToPercentVal ("50"), 0.5f, "Incorrect value!");
    }

    void gainParamTest()
    {
        expectEquals (gainValToString (10.0f), String ("10.00 dB"), "Incorrect dB string!");
        expectEquals (stringToGainVal ("10 dB"), 10.0f, "Incorrect dB value!");
        expectEquals (stringToGainVal ("10"), 10.0f, "Incorrect value!");
    }

    void ratioParamTest()
    {
        expectEquals (ratioValToString (10.0f), String ("10.00 : 1"), "Incorrect ratio string!");
        expectEquals (stringToRatioVal ("10 : 1"), 10.0f, "Incorrect ratio value!");
        expectEquals (stringToRatioVal ("10"), 10.0f, "Incorrect value!");
    }

    void timeParamTest()
    {
        expectEquals (timeMsValToString (10.0f), String ("10.00 ms"), "Incorrect milliseconds string!");
        expectEquals (timeMsValToString (2000.0f), String ("2.00 s"), "Incorrect seconds string!");
        expectEquals (stringToTimeMsVal ("200 ms"), 200.0f, "Incorrect milliseconds value!");
    }

    void floatParamTest()
    {
        expectEquals (floatValToString (10.0f), String ("10.00"), "Incorrect float string!");
        expectEquals (floatValToStringDecimal (10.0f, 1), String ("10.0"), "Incorrect float decimal string!");
        expectEquals (stringToFloatVal ("200"), 200.0f, "Incorrect float value!");
    }

    void runTest() override
    {
        beginTest ("Freq. Param Test");
        freqParamTest();

        beginTest ("Percent Param Test");
        percentParamTest();

        beginTest ("Gain Param Test");
        gainParamTest();

        beginTest ("Ratio Param Test");
        ratioParamTest();

        beginTest ("Time Param Test");
        timeParamTest();

        beginTest ("Float Param Test");
        floatParamTest();
    }
};

static ParamStringsTest paramStringsTest;
