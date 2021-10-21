#include "../test_utils.h"

class BBDTest : public UnitTest
{
public:
    BBDTest() : UnitTest ("BBD Test") {}

    void processDelay (double fs, float delaySamples, float* samples, int numSamples)
    {
        chowdsp::BBD::BBDDelayWrapper<8192> delay;
        delay.prepare ({ fs, (uint32) numSamples, 1 });
        delay.setDelay (delaySamples);

        for (int i = 0; i < numSamples; ++i)
        {
            delay.setFilterFreq (10000.0f);
            delay.pushSample (0, samples[i]);
            samples[i] = delay.popSample (0);
        }
    }

    int findFirstNonZero (const float* bufferPtr, const int numSamples)
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

    void preDelayTest()
    {
        constexpr double fs = 48000.0;
        constexpr int numSamples = 400;

        AudioBuffer<float> buffer (1, numSamples);
        auto bufferPtr = buffer.getWritePointer (0);
        FloatVectorOperations::fill (bufferPtr, 1.0f, numSamples);

        processDelay (fs, (float) numSamples / 2.0f, bufferPtr, numSamples);

        auto firstNonZero = findFirstNonZero (bufferPtr, numSamples);
        expectEquals (firstNonZero, numSamples / 2, "First non-zero sample is incorrect!");
    }

    void postDelayTest()
    {
        constexpr double fs = 48000.0;
        constexpr int numSamples = 2048;

        AudioBuffer<float> buffer (1, numSamples);
        auto bufferPtr = buffer.getWritePointer (0);
        FloatVectorOperations::fill (bufferPtr, 1.0f, numSamples);

        processDelay (fs, (float) numSamples / 2.0f, bufferPtr, numSamples);

        auto maxErr = 0.0f;
        for (int i = numSamples / 2 + 10; i < numSamples; ++i)
        {
            auto err = std::abs (bufferPtr[i] - 1.57f);
            maxErr = jmax (err, maxErr);
        }

        expectLessThan (maxErr, 0.001f, "Maximum error too large!");
    }

    void aaFilterTest()
    {
        constexpr double fs = 48000.0;
        constexpr int numSamples = 2048;
        constexpr auto lenSeconds = (float) numSamples / (float) fs;

        auto runBuffer = [=] (float filterFreq, auto& samples) {
            chowdsp::BBD::BBDDelayWrapper<8192> delay;
            delay.prepare ({ fs, (uint32) numSamples, 1 });
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

        auto lowMag = lowBuffer.getMagnitude (0, numSamples);
        auto highMag = highBuffer.getMagnitude (0, numSamples);

        auto lowErr = std::abs (Decibels::gainToDecibels (lowMag));
        auto highErr = std::abs (Decibels::gainToDecibels (highMag) + 17.0f);

        expectLessThan (lowErr, 0.5f, "Low band too much error!");
        expectLessThan (highErr, 0.5f, "High band too much error!");
    }

    void zeroDelayTest()
    {
        constexpr double fs = 48000.0;
        constexpr int numSamples = 200;

        AudioBuffer<float> buffer (1, numSamples);
        auto bufferPtr = buffer.getWritePointer (0);
        FloatVectorOperations::fill (bufferPtr, 1.0f, numSamples);

        processDelay (fs, 0.0f, bufferPtr, numSamples);

        auto firstNonZero = findFirstNonZero (bufferPtr, numSamples);
        expectLessThan (firstNonZero, 170, "First non-zero sample should be less than 170!");
    }

    void runTest() override
    {
        beginTest ("Pre-Delay Test");
        preDelayTest();

        beginTest ("Post-Delay Test");
        postDelayTest();

        beginTest ("AA-Filter Test");
        aaFilterTest();

        beginTest ("Zero-Delay Test");
        zeroDelayTest();
    }
};

static BBDTest bbdTest;
