#include <CatchUtils.h>
#include <chowdsp_parameters/chowdsp_parameters.h>

using namespace chowdsp::ParamUtils;

TEST_CASE("Param Strings Test", "[plugin][parameters]")
{
    SECTION("Freq. Param Test")
    {
        REQUIRE_MESSAGE (freqValToString (100.0f) == juce::String ("100.00 Hz"), "Incorrect small freq string!");
        REQUIRE_MESSAGE (freqValToString (10000.0f) == juce::String ("10.00 kHz"), "Incorrect large freq string!");
        REQUIRE_MESSAGE (stringToFreqVal ("100") == 100.0f, "Incorrect small freq value!");
        REQUIRE_MESSAGE (stringToFreqVal ("10k") == 10000.0f, "Incorrect large freq value!");
    }

    SECTION("Percent Param Test")
    {
        REQUIRE_MESSAGE (percentValToString (0.5f) == juce::String ("50%"), "Incorrect percent string!");
        REQUIRE_MESSAGE (stringToPercentVal ("50%") == 0.5f, "Incorrect percent value!");
        REQUIRE_MESSAGE (stringToPercentVal ("50") == 0.5f, "Incorrect value!");
    }

    SECTION("Gain Param Test")
    {
        REQUIRE_MESSAGE (gainValToString (10.0f) == juce::String ("10.00 dB"), "Incorrect dB string!");
        REQUIRE_MESSAGE (stringToGainVal ("10 dB") == 10.0f, "Incorrect dB value!");
        REQUIRE_MESSAGE (stringToGainVal ("10") == 10.0f, "Incorrect value!");
    }

    SECTION("Ratio Param Test")
    {
        REQUIRE_MESSAGE (ratioValToString (10.0f) == juce::String ("10.00 : 1"), "Incorrect ratio string!");
        REQUIRE_MESSAGE (stringToRatioVal ("10 : 1") == 10.0f, "Incorrect ratio value!");
        REQUIRE_MESSAGE (stringToRatioVal ("10") == 10.0f, "Incorrect value!");
    }

    SECTION("Time Param Test")
    {
        REQUIRE_MESSAGE (timeMsValToString (10.0f) == juce::String ("10.00 ms"), "Incorrect milliseconds string!");
        REQUIRE_MESSAGE (timeMsValToString (2000.0f) == juce::String ("2.00 s"), "Incorrect seconds string!");
        REQUIRE_MESSAGE (stringToTimeMsVal ("200 ms") == 200.0f, "Incorrect milliseconds value!");
    }

    SECTION("Float Param Test")
    {
        REQUIRE_MESSAGE (floatValToString (10.0f) == juce::String ("10.00"), "Incorrect float string!");
        REQUIRE_MESSAGE (floatValToStringDecimal<1> (10.0f) == juce::String ("10.0"), "Incorrect float decimal string!");
        REQUIRE_MESSAGE (stringToFloatVal ("200") == 200.0f, "Incorrect float value!");
    }
}
