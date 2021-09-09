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
            delay.pushSample (0, samples[i]);
            samples[i] = delay.popSample (0);
        }
    }

    void preDelayTest()
    {
        constexpr double fs = 48000.0;
        constexpr int numSamples = 200;

        AudioBuffer<float> buffer (1, numSamples);
        auto bufferPtr = buffer.getWritePointer (0);
        FloatVectorOperations::fill (bufferPtr, 1.0f, numSamples);

        processDelay (fs, (float) numSamples / 2.0f, bufferPtr, numSamples);

        int firstNonZero = -1;
        for (int i = 0; i < numSamples; ++i)
        {
            if (std::abs (bufferPtr[i]) > 1.0e-6f)
            {
                firstNonZero = i;
                break;
            }
        }

        expectEquals (firstNonZero, numSamples / 2, "First non-zero sample is incorrect!");
    }

    void postDelayTest()
    {
        constexpr double fs = 48000.0;
        constexpr int numSamples = 200;

        auto refBuffer = test_utils::makeSineWave (100.0f, fs, (float) numSamples / (float) fs);
        auto buffer = test_utils::makeSineWave (100.0f, fs, (float) numSamples / (float) fs);
        auto bufferPtr = buffer.getWritePointer (0);

        processDelay (fs, (float) numSamples / 2.0f, bufferPtr, numSamples);

        auto maxErr = 0.0f;
        for (int i = numSamples / 2 + 10; i < numSamples; ++i)
        {
            auto err = std::abs (bufferPtr[i] - refBuffer.getSample (0, i - numSamples / 2));
            maxErr = jmax (err, maxErr);
        }

        for (int i = numSamples / 2; i < numSamples; ++i)
        {
            std::cout << bufferPtr[i] << std::endl;
        }

        std::cout << "Max error: " << maxErr << std::endl;
    }

    void runTest() override
    {
        beginTest ("Pre-Delay Test");
        preDelayTest();

        beginTest ("Post-Delay Test");
        postDelayTest();

        std::string tt;
        std::cin >> tt;

        beginTest ("AA-Filter Test");
        std::cout << "TODO..." << std::endl;
    }
};

static BBDTest bbdTest;
