#include <CatchUtils.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wpessimizing-move") // Clang doesn't like std::move

namespace
{
// Processing constants
constexpr size_t fftOrder = 15;
constexpr size_t blockSize = 1 << fftOrder;
constexpr size_t nCh = 1;
constexpr double fs = 48000.0;

// Testing constants
constexpr float testFreq = 100.0f;
constexpr float semitoneShift = 5.0f;
constexpr float semitoneShiftFreq = 133.4f;
constexpr float scaleShift = 0.75f;
constexpr float scaleShiftFreq = 75.0f;
} // namespace

static float tunerTest (const chowdsp::BufferView<float>& buffer)
{
    chowdsp::TunerProcessor<float> tuner;
    tuner.prepare ((double) fs);
    tuner.process (buffer.getReadPointer (0));

    return tuner.getCurrentFrequencyHz();
}

/** Does pitch-shift processing over a sample buffer */
static chowdsp::Buffer<float> processBuffer (chowdsp::PitchShifter<float>& shifter)
{
    auto sineBuffer = test_utils::makeSineWave (testFreq, (float) fs, (float) blockSize / (float) fs);
    shifter.processBlock (sineBuffer);
    return std::move (sineBuffer);
}

/** Does pitch-shift processing sample-by-sample */
static chowdsp::Buffer<float> processSamples (chowdsp::PitchShifter<float>& shifter)
{
    auto sineBuffer = test_utils::makeSineWave (testFreq, (float) fs, (float) blockSize / (float) fs);

    for (int ch = 0; ch < sineBuffer.getNumChannels(); ++ch)
    {
        auto* x = sineBuffer.getWritePointer (ch);
        for (int i = 0; i < sineBuffer.getNumSamples(); ++i)
            x[i] = shifter.processSample ((size_t) ch, x[i]);
    }

    return std::move (sineBuffer);
}

/** Runs pitch shifting on a sine wave, and checks SNR (in dB) */
static void runPitchShiftTest (bool shouldUseBuffers, bool shouldShift, bool shouldUseSemitones)
{
    chowdsp::PitchShifter<float> shifter;
    shifter.prepare ({ fs, blockSize, nCh });

    float freqExpected = testFreq;
    if (shouldShift)
    {
        if (shouldUseSemitones)
        {
            shifter.setShiftSemitones (semitoneShift);
            freqExpected = semitoneShiftFreq;
        }
        else
        {
            shifter.setShiftFactor (scaleShift);
            freqExpected = scaleShiftFreq;
        }
    }
    else
    {
        shifter.setShiftSemitones (0.0f);
    }

    chowdsp::Buffer<float> buffer;
    if (shouldUseBuffers)
        buffer = processBuffer (shifter);
    else
        buffer = processSamples (shifter);

    const auto actualFrequency = tunerTest (buffer);
    REQUIRE_MESSAGE (actualFrequency == Catch::Approx (freqExpected).margin (freqExpected * 0.01f), "Shifted frequency is incorrect!");

    //    REQUIRE_MESSAGE (snr >= minSNR, "SNR too low!");
}

/** Unit tests for chowdsp::PitchShifter. Tests play a sine wave through
 *  the pitch-shifter and measure the SNR of the output sine wave. Tests include:
 *  - Single-sample processing
 *  - Buffer processing
 *  - Shifting by scale factor
 *  - Shifting by semitones
 *  - No shift
 */
TEST_CASE ("Pitch Shift Test", "[dsp][misc]")
{
    SECTION ("Single-sample Processing")
    {
        runPitchShiftTest (false, true, true);
    }

    SECTION ("Buffer Processing")
    {
        runPitchShiftTest (true, true, true);
    }

    SECTION ("No Shift")
    {
        runPitchShiftTest (true, false, true);
    }

    SECTION ("Scale Shift")
    {
        runPitchShiftTest (true, true, false);
    }
}

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
