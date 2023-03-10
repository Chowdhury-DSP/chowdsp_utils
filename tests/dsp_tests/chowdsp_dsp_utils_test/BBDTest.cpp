#include <CatchUtils.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

static void processDelay (double fs, float delaySamples, float* samples, int numSamples)
{
    chowdsp::BBD::BBDDelayWrapper<8192> delay;
    delay.prepare ({ fs, (juce::uint32) numSamples, 1 });
    delay.setDelay (delaySamples);

    for (int i = 0; i < numSamples; ++i)
    {
        delay.setFilterFreq (10000.0f);
        delay.pushSample (0, samples[i]);
        samples[i] = delay.popSample (0);
    }
    delay.free();
}

static int findFirstNonZero (const float* bufferPtr, const int numSamples)
{
    int firstNonZero = -1;
    for (int i = 0; i < numSamples; ++i)
    {
        if (std::abs (bufferPtr[i]) > 1.0e-6f)
        {
            firstNonZero = i;
            break;
        }
    }

    return firstNonZero;
}

TEST_CASE ("BBD Test", "[dsp][misc]")
{
    SECTION ("Pre-Delay Test")
    {
        constexpr double fs = 48000.0;
        constexpr int numSamples = 400;

        chowdsp::Buffer<float> buffer (1, numSamples);
        auto bufferPtr = buffer.getWritePointer (0);
        juce::FloatVectorOperations::fill (bufferPtr, 1.0f, numSamples);

        processDelay (fs, (float) numSamples / 2.0f, bufferPtr, numSamples);

        auto firstNonZero = findFirstNonZero (bufferPtr, numSamples);
        REQUIRE_MESSAGE (firstNonZero == numSamples / 2, "First non-zero sample is incorrect!");
    }

    SECTION ("Post-Delay Test")
    {
        constexpr double fs = 48000.0;
        constexpr int numSamples = 2048;

        chowdsp::Buffer<float> buffer (1, numSamples);
        auto bufferPtr = buffer.getWritePointer (0);
        juce::FloatVectorOperations::fill (bufferPtr, 1.0f, numSamples);

        processDelay (fs, (float) numSamples / 2.0f, bufferPtr, numSamples);

        auto maxErr = 0.0f;
        for (int i = numSamples / 2 + 10; i < numSamples; ++i)
        {
            auto err = std::abs (bufferPtr[i] - 1.57f);
            maxErr = juce::jmax (err, maxErr);
        }

        REQUIRE_MESSAGE (maxErr < 0.001f, "Maximum error too large!");
    }

    SECTION ("AA-Filter Test")
    {
        constexpr double fs = 48000.0;
        constexpr int numSamples = 2048;
        constexpr auto lenSeconds = (float) numSamples / (float) fs;

        auto runBuffer = [=] (float filterFreq, auto& samples)
        {
            chowdsp::BBD::BBDDelayWrapper<8192> delay;
            delay.prepare ({ fs, (juce::uint32) numSamples, 1 });
            delay.setDelay (1.0f);

            auto* x = samples.getWritePointer (0);
            for (int i = 0; i < numSamples; ++i)
            {
                delay.setFilterFreq (filterFreq);
                delay.pushSample (0, x[i]);
                x[i] = delay.popSample (0);
            }
        };

        auto lowBuffer = test_utils::makeSineWave (100.0f, (float) fs, lenSeconds);
        auto highBuffer = test_utils::makeSineWave (10000.0f, (float) fs, lenSeconds);

        runBuffer (1000.0f, lowBuffer);
        runBuffer (1000.0f, highBuffer);

        auto lowMag = chowdsp::BufferMath::getMagnitude (lowBuffer);
        auto highMag = chowdsp::BufferMath::getMagnitude (highBuffer);

        auto lowErr = std::abs (juce::Decibels::gainToDecibels (lowMag));
        auto highErr = std::abs (juce::Decibels::gainToDecibels (highMag) + 17.0f);

        REQUIRE_MESSAGE (lowErr < 0.5f, "Low band too much error!");
        REQUIRE_MESSAGE (highErr < 0.5f, "High band too much error!");
    }

    SECTION ("Zero-Delay Test")
    {
        constexpr double fs = 48000.0;
        constexpr int numSamples = 200;

        chowdsp::Buffer<float> buffer (1, numSamples);
        auto bufferPtr = buffer.getWritePointer (0);
        juce::FloatVectorOperations::fill (bufferPtr, 1.0f, numSamples);

        processDelay (fs, 0.0f, bufferPtr, numSamples);

        auto firstNonZero = findFirstNonZero (bufferPtr, numSamples);
        REQUIRE_MESSAGE (firstNonZero < 170, "First non-zero sample should be less than 170!");
    }
}
