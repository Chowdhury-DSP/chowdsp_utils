#include <test_utils.h>
#include <TimedUnitTest.h>
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

// FFT smoothing constants
constexpr size_t avgNum = 3;
constexpr size_t negDiff = avgNum / 2;
constexpr size_t posDiff = negDiff + 1;
} // namespace

/** Unit tests for chowdsp::PitchShifter. Tests play a sine wave through
 *  the pitch-shifter and measure the SNR of the output sine wave. Tests include:
 *  - Single-sample processing
 *  - Buffer processing
 *  - Shifting by scale factor
 *  - Shifting by semitones
 *  - No shift
 */
class PitchShiftTest : public TimedUnitTest
{
public:
    PitchShiftTest() : TimedUnitTest ("Pitch Shift Test") {}

    /** Returns frequency domain SNR in dB */
    static float calcSNR (const juce::AudioBuffer<float>& buffer, float freqExpected)
    {
        juce::dsp::FFT fft (fftOrder);

        std::vector<float> fftData (blockSize * 2, 0.0f);
        juce::FloatVectorOperations::copy (fftData.data(), buffer.getReadPointer (0), blockSize);
        fft.performFrequencyOnlyForwardTransform (fftData.data());

        std::vector<float> magnitudes (blockSize);
        auto dBNorm = 3.0f * fftOrder - 6.0f;
        auto scaleNorm = juce::Decibels::decibelsToGain<float> (dBNorm);
        for (size_t i = 0; i < blockSize; ++i)
            magnitudes[i] = std::pow (fftData[i] / scaleNorm, 2.0f);

        auto getMagForFreq = [=] (float freq) -> float {
            auto idx = size_t (((float) blockSize / 2.0f) * freq / (fs / 2.0f));
            // average over a few bins to smooth
            return std::accumulate (&magnitudes[idx - negDiff], &magnitudes[idx + posDiff], 0.0f) / (float) avgNum;
        };

        float noiseAccum = 0.0f;
        float freqInc = (float) fs / (float) blockSize;

        for (float freq = 20.0f; freq < 0.66667f * freqExpected; freq += freqInc)
            noiseAccum += getMagForFreq (freq);

        float signalAccum = getMagForFreq (freqExpected);

        for (float freq = 1.5f * freqExpected; freq < (float) fs * 0.48f; freq += freqInc)
            noiseAccum += getMagForFreq (freq);

        // noiseAccum /= (float) blockSize / 2.0f;
        return juce::Decibels::gainToDecibels (signalAccum / noiseAccum);
    }

    /** Does pitch-shift processing over a sample buffer */
    static juce::AudioBuffer<float> processBuffer (chowdsp::PitchShifter<float>& shifter)
    {
        auto sineBuffer = test_utils::makeSineWave (testFreq, (float) fs, (float) blockSize / (float) fs);
        juce::dsp::AudioBlock<float> sineBlock (sineBuffer);
        juce::dsp::ProcessContextReplacing<float> ctx (sineBlock);
        shifter.process (ctx);

        return std::move (sineBuffer);
    }

    /** Does pitch-shift processing sample-by-sample */
    static juce::AudioBuffer<float> processSamples (chowdsp::PitchShifter<float>& shifter)
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
    void runPitchShiftTest (bool shouldUseBuffers, bool shouldShift, bool shouldUseSemitones, float minSNR)
    {
        chowdsp::PitchShifter<float> shifter;

        juce::dsp::ProcessSpec spec { fs, blockSize, nCh };
        shifter.prepare (spec);

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

        juce::AudioBuffer<float> buffer;
        if (shouldUseBuffers)
            buffer = processBuffer (shifter);
        else
            buffer = processSamples (shifter);

        auto snr = calcSNR (buffer, freqExpected);
        juce::Logger::writeToLog ("SNR: " + juce::String (snr));
        expectGreaterOrEqual (snr, minSNR, "SNR too low!");
    }

    void runTestTimed() override
    {
        beginTest ("Single-sample Processing");
        runPitchShiftTest (false, true, true, 35.0f);

        beginTest ("Buffer Processing");
        runPitchShiftTest (true, true, true, 35.0f);

        beginTest ("No Shift");
        runPitchShiftTest (true, false, true, 35.0f);

        beginTest ("Scale Shift");
        runPitchShiftTest (true, true, false, 35.0f);
    }
};

static PitchShiftTest pitchShiftTest;

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
