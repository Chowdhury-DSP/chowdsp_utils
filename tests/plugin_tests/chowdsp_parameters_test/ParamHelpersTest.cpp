#include <CatchUtils.h>
#include <chowdsp_parameters/chowdsp_parameters.h>

using namespace chowdsp::ParamUtils;

static void checkRange (const juce::NormalisableRange<float>& range, float start, float end, float centre)
{
    constexpr float maxErr = 1.0e-3f;

    REQUIRE_MESSAGE (range.convertFrom0to1 (0.0f) == Catch::Approx { start }.margin (maxErr), "Converting from 0 -> 1 at start is incorrect!");
    REQUIRE_MESSAGE (range.convertFrom0to1 (0.5f) == Catch::Approx { centre }.margin (maxErr), "Converting from 0 -> 1 at centre is incorrect!");
    REQUIRE_MESSAGE (range.convertFrom0to1 (1.0f) == Catch::Approx { end }.margin (maxErr), "Converting from 0 -> 1 at end is incorrect!");

    REQUIRE_MESSAGE (range.convertTo0to1 (start) == Catch::Approx { 0.0f }.margin (maxErr), "Converting to 0 -> 1 at start is incorrect!");
    REQUIRE_MESSAGE (range.convertTo0to1 (centre) == Catch::Approx { 0.5f }.margin (maxErr), "Converting to 0 -> 1 at centre is incorrect!");
    REQUIRE_MESSAGE (range.convertTo0to1 (end) == Catch::Approx { 1.0f }.margin (maxErr), "Converting to 0 -> 1 at end is incorrect!");
}

TEST_CASE ("Param Helpers Test", "[plugin][parameters]")
{
    SECTION ("Create Normalisable Range Test")
    {
        constexpr float start = 0.05f;
        constexpr float end = 1.5f;
        constexpr float centre = 1.35f;

        auto testRange = createNormalisableRange (start, end, centre);
        checkRange (testRange, start, end, centre);
    }

    SECTION ("Create Freq. Param Test")
    {
        constexpr float start = 200.0f;
        constexpr float end = 20000.0f;
        constexpr float centre = 2000.0f;

        chowdsp::Parameters params;
        createFreqParameter (params, "test", "Test", start, end, centre, centre);
        checkRange (params[0]->getNormalisableRange(), start, end, centre);

        REQUIRE_MESSAGE (params[0]->getText (0.5f, 1024) == juce::String ("2.00 kHz"), "kHz value string is incorrect!");
        REQUIRE_MESSAGE (params[0]->getText (0.0f, 1024) == juce::String ("200.00 Hz"), "Hz value string is incorrect!");
    }

    SECTION ("Create Percent Param Test")
    {
        chowdsp::Parameters params;
        createPercentParameter (params, "test", "Test", 0.5f);
        createBipolarPercentParameter (params, "test_bp", "Test");

        REQUIRE_MESSAGE (params[0]->getText (0.5f, 1024) == juce::String ("50%"), "50% value string is incorrect!");
        REQUIRE_MESSAGE (params[0]->getText (0.0f, 1024) == juce::String ("0%"), "0% value string is incorrect!");

        REQUIRE_MESSAGE (params[1]->getText (0.0f, 1024) == juce::String ("-100%"), "-100% value string is incorrect!");
        REQUIRE_MESSAGE (params[1]->getText (0.5f, 1024) == juce::String ("0%"), "0% value string is incorrect!");
        REQUIRE_MESSAGE (params[1]->getText (1.0f, 1024) == juce::String ("100%"), "100% value string is incorrect!");
    }

    SECTION ("Create Gain DB Param Test")
    {
        chowdsp::Parameters params;
        createGainDBParameter (params, "test", "Test", -100.0f, 0.0f, -12.0f, -12.0f);

        REQUIRE_MESSAGE (params[0]->getText (0.5f, 1024) == juce::String ("-12.00 dB"), "-12 dB value string is incorrect!");
        REQUIRE_MESSAGE (params[0]->getText (1.0f, 1024) == juce::String ("0.00 dB"), "0 dB value string is incorrect!");
    }

    SECTION ("Create Time Milliseconds Param Test")
    {
        chowdsp::Parameters params;
        createTimeMsParameter (params, "test", "Test", createNormalisableRange (0.0f, 2000.0f, 200.0f), 0.0f);

        REQUIRE_MESSAGE (params[0]->getText (0.0f, 1024) == juce::String ("0.00 ms"), "0 ms value string is incorrect!");
        REQUIRE_MESSAGE (params[0]->getText (0.5f, 1024) == juce::String ("200.00 ms"), "200 ms value string is incorrect!");
        REQUIRE_MESSAGE (params[0]->getText (1.0f, 1024) == juce::String ("2.00 s"), "2 s value string is incorrect!");
    }

    SECTION ("Create Ratio Param Test")
    {
        chowdsp::Parameters params;
        createRatioParameter (params, "test", "Test", createNormalisableRange (0.5f, 5.0f, 1.0f), 1.0f);

        REQUIRE_MESSAGE (params[0]->getText (0.0f, 1024) == juce::String ("0.50 : 1"), "0.5 : 1 value string is incorrect!");
        REQUIRE_MESSAGE (params[0]->getText (0.5f, 1024) == juce::String ("1.00 : 1"), "1 : 1 value string is incorrect!");
        REQUIRE_MESSAGE (params[0]->getText (1.0f, 1024) == juce::String ("5.00 : 1"), "5 : 1 value string is incorrect!");
    }

    SECTION ("Create Enum Param Test")
    {
        enum Mode
        {
            Mode_1 = 1,
            Mode_2 = 2,
            Mode_3 = 4,
        };

        chowdsp::EnumChoiceParameter<Mode> modeParam ("mode", "Mode", Mode::Mode_1);

        REQUIRE_MESSAGE (static_cast<juce::RangedAudioParameter&> (modeParam).getText (0.0f, 1024) == juce::String ("Mode 1"), "Mode 1 value string is incorrect!");
        REQUIRE_MESSAGE (static_cast<juce::RangedAudioParameter&> (modeParam).getText (0.5f, 1024) == juce::String ("Mode 2"), "Mode 2 value string is incorrect!");
        REQUIRE_MESSAGE (static_cast<juce::RangedAudioParameter&> (modeParam).getText (1.0f, 1024) == juce::String ("Mode 3"), "Mode 3 value string is incorrect!");

        REQUIRE_MESSAGE (modeParam.get() == Mode_1, "Default value is incorrect!");
        modeParam.setValueNotifyingHost (1.0f);
        REQUIRE_MESSAGE (modeParam.get() == Mode_3, "Set value is incorrect!");
        modeParam.setParameterValue (Mode_2);
        REQUIRE_MESSAGE (modeParam.get() == Mode_2, "Set value is incorrect!");
    }

    SECTION ("Create Semitones Param Test")
    {
        chowdsp::SemitonesParameter semitonesContinuousParam { "st_continuous", "Semitones", juce::NormalisableRange { -12.0f, 12.0f }, 0.0f };
        chowdsp::SemitonesParameter semitonesDiscreteParam { "st_discrete", "Semitones", juce::NormalisableRange { -12.0f, 12.0f }, 0.0f, true };

        REQUIRE (static_cast<juce::RangedAudioParameter&> (semitonesContinuousParam).getText (0.5f, 1024) == juce::String ("0.00 st"));
        REQUIRE (static_cast<juce::RangedAudioParameter&> (semitonesDiscreteParam).getText (0.5f, 1024) == juce::String ("0 st"));

        REQUIRE (static_cast<juce::RangedAudioParameter&> (semitonesContinuousParam).getText (0.0f, 1024) == juce::String ("-12.00 st"));
        REQUIRE (static_cast<juce::RangedAudioParameter&> (semitonesDiscreteParam).getText (0.0f, 1024) == juce::String ("-12 st"));

        REQUIRE (static_cast<juce::RangedAudioParameter&> (semitonesContinuousParam).getText (1.0f, 1024) == juce::String ("+12.00 st"));
        REQUIRE (static_cast<juce::RangedAudioParameter&> (semitonesDiscreteParam).getText (1.0f, 1024) == juce::String ("+12 st"));
    }
}
