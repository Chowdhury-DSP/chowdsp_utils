#include "test_utils.h"
#include <JuceHeader.h>

namespace Constants
{
constexpr float fs = 48000.0f;
constexpr float lowGain = 0.5f;
constexpr float highGain = 2.0f;
constexpr float fc = 1000.0f;
constexpr float maxError = 0.1f;
} // namespace Constants

/** Unit tests for chowdsp::ShelfFilter. Checks for correct gain at:
 *   - All frequencies when high gain == low gain
 *   - Low frequencies cut by -6 dB
 *   - High frequencies boosted by +6 dB
 *   - Transition frequency, stable at +0 dB
 */
class ShelfFilterTest : public UnitTest
{
public:
    ShelfFilterTest() : UnitTest ("Shelf Filter Test") {}

    void plainGainTest()
    {
        chowdsp::ShelfFilter shelfFilter;
        shelfFilter.reset();
        shelfFilter.calcCoefs (2.0f, 2.0f, Constants::fc, Constants::fs);

        auto buffer = test_utils::makeNoise (Constants::fs, 1.0f);
        const int numSamples = buffer.getNumSamples();
        auto refMag = buffer.getRMSLevel (0, 0, numSamples);

        shelfFilter.processBlock (buffer.getWritePointer (0), numSamples);
        auto mag = buffer.getRMSLevel (0, 0, numSamples);

        expectWithinAbsoluteError (mag / refMag, 2.0f, (float) 1.0e-6, "Incorrect behavior when filter reduces to a simple gain.");
    }

    void boostCutTest()
    {
        chowdsp::ShelfFilter shelfFilter;
        shelfFilter.calcCoefs (Constants::lowGain, Constants::highGain, Constants::fc, Constants::fs);

        auto testFrequency = [=, &shelfFilter] (float freq, float expGain, const String& message) {
            auto buffer = test_utils::makeSineWave (freq, Constants::fs, 1.0f);

            shelfFilter.reset();
            shelfFilter.processBlock (buffer.getWritePointer (0), buffer.getNumSamples());

            auto mag = buffer.getMagnitude (0, buffer.getNumSamples());
            expectWithinAbsoluteError (mag, expGain, Constants::maxError, message);
        };

        testFrequency (10.0f, Constants::lowGain, "Incorrect gain at low frequencies.");
        testFrequency (Constants::fc, 1.0f, "Incorrect gain at transition frequency.");
        testFrequency (20000.0f, Constants::highGain, "Incorrect gain at high frequencies.");
    }

    void runTest() override
    {
        beginTest ("Plain Gain Test");
        plainGainTest();

        beginTest ("Boost/Cut Test");
        boostCutTest();
    }
};

static ShelfFilterTest sfTest;
