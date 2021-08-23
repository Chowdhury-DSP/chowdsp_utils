#include "test_utils.h"

class ConvolutionTest : public UnitTest
{
public:
    ConvolutionTest() : UnitTest ("Convolution Test") {}

    void createTestIR (std::vector<float>& ir, size_t size)
    {
        const auto halfSize = size / 2;

        for (size_t i = 0; i < size; ++i)
            ir[i] = 1.0f - std::abs ((float) ((int) i - (int) halfSize) / (float) halfSize);
    }

    void accuracyTest (bool zeroLatency, float tolerance)
    {
        constexpr size_t irSize = 256;

        std::vector<float> testIR (irSize);
        createTestIR (testIR, irSize);

        chowdsp::ConvolutionEngine engine (testIR.data(), irSize, irSize);

        std::vector<float> testOutput (zeroLatency ? irSize : 2 * irSize);
        testOutput[0] = 1.0f;

        if (zeroLatency)
        {
            engine.processSamples (testOutput.data(), testOutput.data(), irSize);
        }
        else
        {
            for (size_t ptr = 0; ptr < testOutput.size(); ptr += irSize)
                engine.processSamplesWithAddedLatency (testOutput.data() + ptr, testOutput.data() + ptr, irSize);
        }

        auto checkAccuracy = [=] (float* output) {
            for (size_t i = 0; i < irSize; ++i)
            {
                auto error = std::abs (output[i] - testIR[i]);
                expectLessThan (error, tolerance, "Convolution output is not accurate!");
            }
        };

        checkAccuracy (testOutput.data() + (zeroLatency ? 0 : irSize));
    }

    void smoothTransferTest()
    {
        constexpr float freq = 25.0f;
        constexpr float fs = 48000.0f;
        constexpr size_t irSize = 1024;

        std::vector<float> testIR (irSize, 0.0f);
        testIR[irSize / 2] = 1.0f;

        chowdsp::ConvolutionEngine engine (testIR.data(), irSize, irSize);

        auto sineBuffer = test_utils::makeSineWave (freq, fs, 2.0f);
        const auto sineLength = sineBuffer.getNumSamples();
        auto* bufferPtr = sineBuffer.getWritePointer (0);

        size_t samplePtr = 0;
        for (; samplePtr < (size_t) fs; samplePtr += irSize)
            engine.processSamples (&bufferPtr[samplePtr], &bufferPtr[samplePtr], irSize);

        std::vector<float> testIR2 (irSize, 0.0f);
        testIR2[irSize / 2 + 1] = 1.0f;

        chowdsp::IRTransfer irTransfer (engine);
        irTransfer.setNewIR (testIR2.data());
        irTransfer.transferIR (engine);

        for (; samplePtr + irSize <= (size_t) sineLength; samplePtr += irSize)
            engine.processSamples (&bufferPtr[samplePtr], &bufferPtr[samplePtr], irSize);

        if (samplePtr < (size_t) sineLength)
            engine.processSamples (&bufferPtr[samplePtr], &bufferPtr[samplePtr], (size_t) sineLength - samplePtr);

        float maxDiff = 0.0f;
        for (int i = (int) irSize; i < sineLength; ++i)
        {
            auto curDiff = std::abs (bufferPtr[i] - bufferPtr[i - 1]);
            maxDiff = jmax (maxDiff, curDiff);
        }

        expectLessThan (maxDiff, 0.5f, "IR Transfer is not smooth enough!");
    }

    void runTest() override
    {
        beginTest ("Accuracy Test");
        accuracyTest (false, 1.0e-6f);

        beginTest ("Accuracy Test (Zero-Latency)");
        accuracyTest (true, 1.0e-6f);

        beginTest ("Smooth Transfer Test");
        smoothTransferTest();
    }
};

static ConvolutionTest convTest;
