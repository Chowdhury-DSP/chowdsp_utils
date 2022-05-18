#include <test_utils.h>
#include <TimedUnitTest.h>
#include <chowdsp_filters/chowdsp_filters.h>

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
class ShelfFilterTest : public TimedUnitTest
{
public:
    ShelfFilterTest() : TimedUnitTest ("Shelf Filter Test", "Filters") {}

    template <typename T, typename NumericType = chowdsp::SampleTypeHelpers::NumericType<T>>
    void plainGainTest()
    {
        chowdsp::ShelfFilter<T> shelfFilter;
        shelfFilter.reset();
        shelfFilter.calcCoefs ((T) 2, (T) 2, (T) Constants::fc, Constants::fs);

        auto buffer = test_utils::makeNoise (Constants::fs, (NumericType) 1);
        const int numSamples = buffer.getNumSamples();
        auto refMag = buffer.getRMSLevel (0, 0, numSamples);

        juce::HeapBlock<char> dataBlock;
        auto block = test_utils::bufferToBlock<T> (dataBlock, buffer);

        shelfFilter.processBlock (block.getChannelPointer (0), numSamples);

        test_utils::blockToBuffer<T> (buffer, block);
        auto mag = buffer.getRMSLevel (0, 0, numSamples);

        expectWithinAbsoluteError (mag / refMag, 2.0f, (float) 1.0e-6, "Incorrect behavior when filter reduces to a simple gain.");
    }

    template <typename T, typename NumericType = chowdsp::SampleTypeHelpers::NumericType<T>>
    void boostCutTest()
    {
        chowdsp::ShelfFilter<T> shelfFilter;
        shelfFilter.calcCoefs ((T) Constants::lowGain, (T) Constants::highGain, (T) Constants::fc, Constants::fs);

        auto testFrequency = [=, &shelfFilter] (float freq, float expGain, const juce::String& message) {
            auto buffer = test_utils::makeSineWave<> (freq, Constants::fs, (NumericType) 1);

            juce::HeapBlock<char> dataBlock;
            auto block = test_utils::bufferToBlock<T> (dataBlock, buffer);

            shelfFilter.reset();
            shelfFilter.processBlock (block.getChannelPointer (0), buffer.getNumSamples());

            test_utils::blockToBuffer<T> (buffer, block);
            auto mag = buffer.getMagnitude (0, buffer.getNumSamples());
            expectWithinAbsoluteError (mag, expGain, Constants::maxError, message);
        };

        testFrequency (10.0f, Constants::lowGain, "Incorrect gain at low frequencies.");
        testFrequency (Constants::fc, 1.0f, "Incorrect gain at transition frequency.");
        testFrequency (20000.0f, Constants::highGain, "Incorrect gain at high frequencies.");
    }

    void runTestTimed() override
    {
        beginTest ("Plain Gain Test");
        plainGainTest<float>();

        beginTest ("Plain Gain SIMD Test");
        plainGainTest<xsimd::batch<float>>();

        beginTest ("Boost/Cut Test");
        boostCutTest<float>();

        beginTest ("Boost/Cut SIMD Test");
        boostCutTest<xsimd::batch<float>>();
    }
};

static ShelfFilterTest sfTest;
