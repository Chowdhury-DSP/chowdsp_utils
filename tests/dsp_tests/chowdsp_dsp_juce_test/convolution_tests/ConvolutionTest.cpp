#include <CatchUtils.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

static void createTestIR (std::vector<float>& ir, size_t size)
{
    const auto halfSize = size / 2;

    for (size_t i = 0; i < size; ++i)
        ir[i] = 1.0f - std::abs ((float) ((int) i - (int) halfSize) / (float) halfSize);
}

static void checkAccuracy (const float* output, const float* testIR, int irSize, float tolerance)
{
    for (int i = 0; i < irSize; ++i)
        REQUIRE_MESSAGE (output[i] == Catch::Approx { testIR[i] }.margin (tolerance), "Convolution output is not accurate!");
}

static void accuracyTest (bool zeroLatency, float tolerance)
{
    constexpr size_t irSize = 256;

    std::vector<float> testIR (irSize);
    createTestIR (testIR, irSize);

    chowdsp::ConvolutionEngine engine (irSize, irSize, testIR.data());

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

    checkAccuracy (testOutput.data() + (zeroLatency ? 0 : irSize), testIR.data(), irSize, tolerance);
}

TEST_CASE ("Convolution Test", "[dsp][convolution]")
{
    SECTION ("Accuracy Test")
    {
        accuracyTest (false, 1.0e-6f);
    }

    SECTION ("Accuracy Test (Zero-Latency)")
    {
        accuracyTest (true, 1.0e-6f);
    }

    SECTION ("Smooth Transfer Test")
    {
        static constexpr float freq = 25.0f;
        static constexpr float fs = 48000.0f;
        constexpr size_t irSize = 1024;

        std::vector<float> testIR (irSize, 0.0f);
        testIR[irSize / 2] = 1.0f;

        chowdsp::ConvolutionEngine engine (irSize, irSize);
        engine.setNewIR (testIR.data());

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
            maxDiff = juce::jmax (maxDiff, curDiff);
        }

        REQUIRE_MESSAGE (maxDiff < 0.5f, "IR Transfer is not smooth enough!");
    }

    SECTION ("Move Construction Test")
    {
        static constexpr size_t irSize = 1024;

        std::vector<float> testIR (irSize, 0.0f);
        testIR[irSize / 2] = 1.0f;

        chowdsp::ConvolutionEngine engine (irSize, irSize);
        engine.setNewIR (testIR.data());

        auto testProcess = [&] (auto& eng)
        {
            std::vector<float> testOutput (2 * irSize);
            testOutput[0] = 1.0f;

            for (size_t ptr = 0; ptr < testOutput.size(); ptr += irSize)
                eng.processSamplesWithAddedLatency (testOutput.data() + ptr, testOutput.data() + ptr, irSize);

            checkAccuracy (testOutput.data() + (int) irSize, testIR.data(), (int) irSize, 1.0e-6f);
        };

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wself-move")
        {
            engine.reset();
            engine = std::move (engine); // not a good idea to move into yourself, but just in case someone does it...
            testProcess (engine);
        }
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        {
            chowdsp::ConvolutionEngine engineTest (1, 1);
            engine.reset();
            engineTest = std::move (engine);
            testProcess (engineTest);
        }
    }
}
