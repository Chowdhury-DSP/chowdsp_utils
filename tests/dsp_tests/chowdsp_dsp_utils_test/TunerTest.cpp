#include <CatchUtils.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

static void tunerTest (float freq, float fs, float maxError)
{
    chowdsp::TunerProcessor<float> tuner;
    tuner.prepare ((double) fs);

    auto bufferSize = tuner.getAutocorrelationSize();
    auto buffer = test_utils::makeSineWave (freq, fs, bufferSize);
    tuner.process (buffer.getReadPointer (0));

    REQUIRE_MESSAGE (tuner.getCurrentFrequencyHz() == Catch::Approx (freq).margin (maxError), "Tuner frequency reading is incorrect!");
}

static void freq2NotesCentsTest (int noteNum, double cents)
{
    auto getMidiNoteInHertz = [] (const int noteNumber, const double frequencyOfA)
    {
        return frequencyOfA * std::pow (2.0, (noteNumber - 69) / 12.0);
    };

    const auto baseFreq = getMidiNoteInHertz (noteNum, 440.0);
    const auto adjustedFreq = baseFreq * std::pow (2.0, cents / 1200.0);
    auto [resNoteNum, resCents] = chowdsp::TuningHelpers::frequencyHzToNoteAndCents (adjustedFreq);

    REQUIRE_MESSAGE (resNoteNum == noteNum, "Note number is incorrect!");
    REQUIRE_MESSAGE (resCents == Catch::Approx (cents).margin (0.01), "Cents is incorrect!");
}

TEST_CASE ("Tuner Test", "[dsp][misc]")
{
    SECTION ("Silence Test")
    {
        constexpr float fs = 48000.0f;
        chowdsp::TunerProcessor<float> tuner;
        tuner.prepare ((double) fs);

        auto bufferSize = tuner.getAutocorrelationSize();
        auto buffer = test_utils::makeSineWave (100.0f, fs, bufferSize);
        chowdsp::BufferMath::applyGain (buffer, 0.001f);
        tuner.process (buffer.getReadPointer (0));

        REQUIRE_MESSAGE (juce::approximatelyEqual (tuner.getCurrentFrequencyHz(), 1.0f), "Tuner frequency should read 1.0 Hz for silence!");
    }

    SECTION ("100 Hz Test")
    {
        tunerTest (100.0f, 48000.0f, 0.25f);
    }

    SECTION ("Low Freq. Hz Test")
    {
        tunerTest (42.5f, 96000.0f, 0.25f);
    }

    SECTION ("High Freq. Test")
    {
        tunerTest (900.0f, 22050.0f, 20.0f);
    }

    SECTION ("Freq to Note/Cents Test")
    {
        freq2NotesCentsTest (69, 0.0);
        freq2NotesCentsTest (42, 2.0);
        freq2NotesCentsTest (72, 25.0);
        freq2NotesCentsTest (54, -25.0);
        freq2NotesCentsTest (101, 2.5);
        freq2NotesCentsTest (57, -41.0);
    }
}
