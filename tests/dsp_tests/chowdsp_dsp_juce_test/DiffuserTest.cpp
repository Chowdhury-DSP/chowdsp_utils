#include <TimedUnitTest.h>
#include <chowdsp_reverb/chowdsp_reverb.h>

class DiffuserTest : public TimedUnitTest
{
public:
    DiffuserTest() : TimedUnitTest ("Diffuser Test") {}

    static constexpr int nChannels = 4;
    static constexpr int nStages = 4;
    using TestDiffuser = chowdsp::Reverb::Diffuser<float, nChannels, chowdsp::DelayLineInterpolationTypes::None, 1 << 13>;

    void energyPreservingTest()
    {
        constexpr float fs = 48000.0f;
        constexpr float diffusionTimeMs = 100.0f;

        chowdsp::Reverb::DiffuserChain<nStages, TestDiffuser> diffuserChain;
        diffuserChain.prepare ((double) fs);
        diffuserChain.setDiffusionTimeMs (diffusionTimeMs);

        std::vector<float> data (nStages * int (diffusionTimeMs * 0.001f * fs), 0.0f);
        data[0] = 1.0f;

        for (auto& x : data)
        {
            float inVec[nChannels] {};
            std::fill (inVec, inVec + nChannels, x);
            auto* outVec = diffuserChain.process (inVec);
            x = chowdsp::FloatVectorOperations::accumulate (outVec, nChannels);
        }

        const auto actualRMS = chowdsp::FloatVectorOperations::computeRMS (data.data(), (int) data.size());
        const auto expRMS = std::sqrt ((float) nChannels / (float) data.size());
        expectWithinAbsoluteError (actualRMS, expRMS, 1.0e-3f, "Energy is not preserved!");
    }

    void resetTest()
    {
        constexpr float fs = 48000.0f;
        constexpr float diffusionTimeMs = 100.0f;

        chowdsp::Reverb::DiffuserChain<nStages, TestDiffuser> diffuserChain;
        diffuserChain.prepare ((double) fs);
        diffuserChain.setDiffusionTimeMs (diffusionTimeMs);

        std::vector<float> data (100, 0.0f);
        data[0] = 1.0f;

        for (auto& x : data)
        {
            float inVec[nChannels] {};
            std::fill (inVec, inVec + nChannels, x);
            diffuserChain.process (inVec);
        }

        diffuserChain.reset();

        float sumAfterReset = 0.0f;
        for (int i = 0; i < 5 * (int) fs; ++i)
        {
            float inVec[nChannels] {};
            auto* outVec = diffuserChain.process (inVec);
            sumAfterReset += chowdsp::FloatVectorOperations::accumulate (outVec, nChannels);
        }

        expectEquals (sumAfterReset, 0.0f, "State was not cleared after reset!");
    }

    void runTestTimed() override
    {
        beginTest ("Energy Preserving Test");
        energyPreservingTest();

        beginTest ("Reset Test");
        resetTest();
    }
};

static DiffuserTest diffuserTest;
