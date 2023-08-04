#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wfloat-equal")
#include <CatchUtils.h>

TEMPLATE_TEST_CASE ("Level Detector Test", "[dsp][misc]", float, double)
{
    SECTION ("Single-Channel Test")
    {
        static constexpr double sampleRate = 48000.0;
        static constexpr int numSamples = 50000;

        chowdsp::Buffer<TestType> testBuffer { 1, numSamples };
        juce::FloatVectorOperations::fill (testBuffer.getWritePointer (0), (TestType) 0, 10000);
        juce::FloatVectorOperations::fill (testBuffer.getWritePointer (0) + 10000, (TestType) 1, 10000);
        juce::FloatVectorOperations::fill (testBuffer.getWritePointer (0) + 20000, (TestType) 0, 10000);
        juce::FloatVectorOperations::fill (testBuffer.getWritePointer (0) + 30000, (TestType) -2, 10000);
        juce::FloatVectorOperations::fill (testBuffer.getWritePointer (0) + 40000, (TestType) 0, 10000);

        chowdsp::LevelDetector<TestType> testLevelDetector;
        testLevelDetector.prepare ({ sampleRate, (uint32_t) numSamples, 1 });
        testLevelDetector.setParameters (5.0f, 50.0f);
        testLevelDetector.processBlock (testBuffer);

        using chowdsp::SIMDUtils::all;
        const auto* testData = testBuffer.getReadPointer (0);
        int n = 0;
        for (; n < 1000; ++n)
        {
            REQUIRE (all (testData[n] == (TestType) 0));
        }
        for (; n < 2000; ++n)
        {
            REQUIRE (all (testData[n] >= testData[n - 1]));
            REQUIRE (all (testData[n] <= (TestType) 1));
        }
        for (; n < 3000; ++n)
        {
            REQUIRE (all (testData[n] <= testData[n - 1]));
            REQUIRE (all (testData[n] >= (TestType) 0));
        }
        for (; n < 4000; ++n)
        {
            REQUIRE (all (testData[n] >= testData[n - 1]));
            REQUIRE (all (testData[n] <= (TestType) 2));
        }
        for (; n < 5000; ++n)
        {
            REQUIRE (all (testData[n] <= testData[n - 1]));
            REQUIRE (all (testData[n] >= (TestType) 0));
        }
    }

    SECTION ("Multi-Channel Test")
    {
        static constexpr double sampleRate = 48000.0;
        static constexpr int numSamples = 50000;
        static constexpr int numChannels = 4;

        chowdsp::Buffer<TestType> testBuffer { numChannels, numSamples };
        for (int ch = 0; ch < numChannels; ++ch)
        {
            juce::FloatVectorOperations::fill (testBuffer.getWritePointer (ch), (TestType) 0, 10000);
            juce::FloatVectorOperations::fill (testBuffer.getWritePointer (ch) + 10000, (TestType) 1, 10000);
            juce::FloatVectorOperations::fill (testBuffer.getWritePointer (ch) + 20000, (TestType) 0, 10000);
            juce::FloatVectorOperations::fill (testBuffer.getWritePointer (ch) + 30000, (TestType) -2, 10000);
            juce::FloatVectorOperations::fill (testBuffer.getWritePointer (ch) + 40000, (TestType) 0, 10000);
        }

        chowdsp::LevelDetector<TestType> testLevelDetector;
        testLevelDetector.prepare ({ sampleRate, (uint32_t) numSamples, (uint32_t) numChannels });
        testLevelDetector.setParameters (5.0f, 50.0f);
        testLevelDetector.processBlock (testBuffer);

        using chowdsp::SIMDUtils::all;
        const auto* testData = testBuffer.getReadPointer (0);
        int n = 0;
        for (; n < 1000; ++n)
        {
            REQUIRE (all (testData[n] == (TestType) 0));
        }
        for (; n < 2000; ++n)
        {
            REQUIRE (all (testData[n] >= testData[n - 1]));
            REQUIRE (all (testData[n] <= (TestType) 1));
        }
        for (; n < 3000; ++n)
        {
            REQUIRE (all (testData[n] <= testData[n - 1]));
            REQUIRE (all (testData[n] >= (TestType) 0));
        }
        for (; n < 4000; ++n)
        {
            REQUIRE (all (testData[n] >= testData[n - 1]));
            REQUIRE (all (testData[n] <= (TestType) 2));
        }
        for (; n < 5000; ++n)
        {
            REQUIRE (all (testData[n] <= testData[n - 1]));
            REQUIRE (all (testData[n] >= (TestType) 0));
        }
    }
}

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
