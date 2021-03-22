#include "../test_utils.h"
#include <JuceHeader.h>

namespace
{
constexpr float fs = 44100.0f;
constexpr float modeDecay = 0.01f;
constexpr float modeAmp = 0.1f;
} // namespace

/** Unit tests for chowdsp::ModalFilter. Tests include:
 *   - Check that filter resonates at center frequency
 *   - Check that filter damps frequencies other than center frequency
 *   - Check that filter has correct decay time
 */
class ModalFilterTest : public UnitTest
{
public:
    ModalFilterTest() : UnitTest ("Modal Filter Test") {}

    void onFreqSineTest()
    {
        constexpr float testFreq = 100.0f;
        auto buffer = test_utils::makeSineWave (testFreq, fs, 1.0f);
        auto refMag = Decibels::gainToDecibels (buffer.getMagnitude (0, buffer.getNumSamples()));

        chowdsp::ModalFilter<float> filter;
        filter.prepare (fs);
        filter.setAmp (modeAmp);
        filter.setDecay (modeDecay);
        filter.setFreq (testFreq);
        filter.processBlock (buffer.getWritePointer (0), buffer.getNumSamples());

        auto mag = Decibels::gainToDecibels (buffer.getMagnitude (0, buffer.getNumSamples()));
        expectGreaterThan (mag - refMag, 6.0f, "Modal filter is not resonating at correct frequency.");
    }

    void offFreqSineTest()
    {
        constexpr float testFreq1 = 100.0f;
        constexpr float testFreq2 = 10000.0f;
        auto buffer = test_utils::makeSineWave (testFreq1, fs, 1.0f);
        auto refMag = Decibels::gainToDecibels (buffer.getMagnitude (0, buffer.getNumSamples()));

        chowdsp::ModalFilter<float> filter;
        filter.prepare (fs);
        filter.setAmp (modeAmp);
        filter.setDecay (modeDecay);
        filter.setFreq (testFreq2);
        filter.processBlock (buffer.getWritePointer (0), buffer.getNumSamples());

        auto mag = Decibels::gainToDecibels (buffer.getMagnitude (0, buffer.getNumSamples()));
        expectLessThan (mag - refMag, -24.0f, "Modal filter is resonating at an incorrect frequency.");
    }

    void decayTimeTest()
    {
        auto buffer = test_utils::makeImpulse (1.0f, fs, 1.0f);

        chowdsp::ModalFilter<float> filter;
        filter.prepare (fs);
        filter.setAmp (1.0f);
        filter.setDecay (0.5f);
        filter.setFreq (100.0f);
        filter.processBlock (buffer.getWritePointer (0), buffer.getNumSamples());

        auto mag = Decibels::gainToDecibels (buffer.getMagnitude (int (fs * 0.5f), int (fs * 0.1f)));
        expectWithinAbsoluteError (mag, -60.0f, 1.0f, "Incorrect decay time.");
    }

    void runTest() override
    {
        beginTest ("Resonant Frequency Sine Test");
        onFreqSineTest();

        beginTest ("Damped Frequency Sine Test");
        offFreqSineTest();

        beginTest ("Decay Time Test");
        decayTimeTest();
    }
};

static ModalFilterTest mfTest;
