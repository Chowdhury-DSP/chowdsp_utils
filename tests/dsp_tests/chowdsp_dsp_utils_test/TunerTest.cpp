#include <TimedUnitTest.h>
#include <test_utils.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

class TunerTest : public TimedUnitTest
{
public:
    TunerTest() : TimedUnitTest ("Tuner Test")
    {
    }

    void silenceTest()
    {
        constexpr float fs = 48000.0f;
        chowdsp::TunerProcessor<float> tuner;
        tuner.prepare ((double) fs);

        auto bufferSize = tuner.getAutocorrelationSize();
        auto buffer = test_utils::makeSineWave (100.0f, fs, bufferSize);
        buffer.applyGain (0.001f);
        tuner.process (buffer.getReadPointer (0));

        expectEquals (tuner.getCurrentFrequencyHz(), 1.0f, "Tuner frequency should read 1.0 Hz for silence!");
    }

    void tunerTest (float freq, float fs, float maxError)
    {
        chowdsp::TunerProcessor<float> tuner;
        tuner.prepare ((double) fs);

        auto bufferSize = tuner.getAutocorrelationSize();
        auto buffer = test_utils::makeSineWave (freq, fs, bufferSize);
        tuner.process (buffer.getReadPointer (0));

        expectWithinAbsoluteError (tuner.getCurrentFrequencyHz(), freq, maxError, "Tuner frequency reading is incorrect!");
    }

    void freq2NotesCentsTest (int noteNum, double cents)
    {
        const auto baseFreq = juce::MidiMessage::getMidiNoteInHertz (noteNum);
        const auto adjustedFreq = baseFreq * std::pow (2.0, cents / 1200.0);
        auto [resNoteNum, resCents] = chowdsp::TuningHelpers::frequencyHzToNoteAndCents (adjustedFreq);

        expectEquals (resNoteNum, noteNum, "Note number is incorrect!");
        expectWithinAbsoluteError (resCents, cents, 0.01, "Cents is incorrect!");
    }

    void runTestTimed() override
    {
        beginTest ("Silence Test");
        silenceTest();

        beginTest ("100 Hz Test");
        tunerTest (100.0f, 48000.0f, 0.25f);

        beginTest ("Low Freq. Hz Test");
        tunerTest (42.5f, 96000.0f, 0.25f);

        beginTest ("High Freq. Test");
        tunerTest (900.0f, 22050.0f, 20.0f);

        beginTest ("Freq to Note/Cents Test");
        freq2NotesCentsTest (69, 0.0);
        freq2NotesCentsTest (42, 2.0);
        freq2NotesCentsTest (72, 25.0);
        freq2NotesCentsTest (54, -25.0);
        freq2NotesCentsTest (101, 2.5);
        freq2NotesCentsTest (57, -41.0);
    }
};

static TunerTest tunerTest;
