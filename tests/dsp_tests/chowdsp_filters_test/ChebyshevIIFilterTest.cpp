#include <test_utils.h>
#include <TimedUnitTest.h>
#include <chowdsp_filters/chowdsp_filters.h>

namespace Constants
{
constexpr float fs = 48000.0f;
constexpr float fc = 1000.0f;
} // namespace Constants

class ChebyshevIIFilterTest : public TimedUnitTest
{
public:
    ChebyshevIIFilterTest() : TimedUnitTest ("Chebyshev II Filter Test", "Filters") {}

    using FilterType = chowdsp::ChebyshevFilterType;

    template <typename T, typename Filter>
    void testFilter (Filter& filt, std::vector<float> freqs, std::vector<float> mags, std::vector<float> errs, const juce::StringArray& messages)
    {
        auto testFrequency = [=, &filt] (float freq, float expGain, float err, const juce::String& message) {
            auto buffer = test_utils::makeSineWave (freq, Constants::fs, 1.0f);

            juce::HeapBlock<char> dataBlock;
            auto block = test_utils::bufferToBlock<T> (dataBlock, buffer);

            filt.reset();
            filt.processBlock (block);

            test_utils::blockToBuffer<float> (buffer, block);
            auto halfBlock = buffer.getNumSamples() / 2;
            auto mag = juce::Decibels::gainToDecibels (buffer.getMagnitude (halfBlock, halfBlock));

            if (err < 0.0f)
                expectLessOrEqual (mag, expGain, message);
            else
                expectWithinAbsoluteError (mag, expGain, err, message);
        };

        for (size_t i = 0; i < freqs.size(); ++i)
            testFrequency (freqs[i], mags[i], errs[i], "Incorrect gain at " + messages[(int) i] + " frequency.");
    }

    template <typename T>
    void lowpassTest()
    {
        { // Chebyshev cutoff frequency
            chowdsp::ChebyshevIIFilter<8, FilterType::Lowpass, 60, false, T> filter;
            filter.calcCoefs (Constants::fc, (T) 1 / juce::MathConstants<T>::sqrt2, Constants::fs);

            testFilter<T> (filter,
                           { 100.0f, Constants::fc, 4 * Constants::fc },
                           { 0.0f, -60.0f, -60.0f },
                           { 0.005f, 1.0f, -1.0f },
                           { "passband", "cutoff", "stopband" });
        }

        { // natural cutoff frequency
            chowdsp::ChebyshevIIFilter<8, FilterType::Lowpass, 60, true, T> filter;
            filter.calcCoefs (Constants::fc, (T) 1 / juce::MathConstants<T>::sqrt2, Constants::fs);

            testFilter<T> (filter,
                           { 100.0f, Constants::fc, 2 * Constants::fc },
                           { 0.0f, -3.0f, -60.0f },
                           { 0.005f, 0.01f, 1.0f },
                           { "passband", "cutoff", "stopband" });
        }
    }

    template <typename T>
    void highpassTest()
    {
        { // Chebyshev cutoff frequency
            chowdsp::ChebyshevIIFilter<8, FilterType::Highpass, 60, false, T> filter;
            filter.calcCoefs (Constants::fc, (T) 1 / juce::MathConstants<T>::sqrt2, Constants::fs);

            testFilter<T> (filter,
                           { 10000.0f, Constants::fc, 0.25f * Constants::fc },
                           { 0.0f, -60.0f, -60.0f },
                           { 0.025f, 4.0f, -1.0f },
                           { "passband", "cutoff", "stopband" });
        }

        { // natural cutoff frequency
            chowdsp::ChebyshevIIFilter<8, FilterType::Highpass, 60, true, T> filter;
            filter.calcCoefs (Constants::fc, (T) 1 / juce::MathConstants<T>::sqrt2, Constants::fs);

            testFilter<T> (filter,
                           { 10000.0f, Constants::fc, 0.5f * Constants::fc },
                           { 0.0f, -3.0f, -60.0f },
                           { 0.025f, 0.01f, 1.0f },
                           { "passband", "cutoff", "stopband" });
        }
    }

    void runTestTimed() override
    {
        beginTest ("Lowpass");
        lowpassTest<float>();

        beginTest ("Highpass");
        highpassTest<float>();
    }
};

static ChebyshevIIFilterTest chebyIIFilterTest;
