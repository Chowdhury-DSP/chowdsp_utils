#include <TimedUnitTest.h>

namespace Constants
{
constexpr float sampleRate = 48000.0f;
constexpr int blockSize = 8;
constexpr float cutoffFreq = 200.0f;
constexpr float filterQ = 0.7071f;
constexpr float filterGain = 2.0f;
constexpr float data[blockSize] = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
} // namespace Constants

class ModFilterWrapperTest : public TimedUnitTest
{
public:
    ModFilterWrapperTest() : TimedUnitTest ("Mod Filter Wrapper Test", "Filters") {}

    void peakingFilterTest()
    {
        using namespace Constants;
        using FilterType = chowdsp::PeakingFilter<float>;
        FilterType originalFilter;
        originalFilter.reset();
        originalFilter.calcCoefs (cutoffFreq, filterQ, filterGain, sampleRate);

        float originalFilterData[blockSize] {};
        for (int n = 0; n < blockSize; ++n)
            originalFilterData[n] = originalFilter.processSample (data[n]);

        chowdsp::ModFilterWrapper<FilterType> modFilter;
        modFilter.prepare ({ sampleRate, (uint32) blockSize, 1 });
        modFilter.calcCoefs (cutoffFreq, filterQ, filterGain, sampleRate);

        float modFilterData[blockSize] {};
        std::copy (data, data + blockSize, modFilterData);

        float* modFilterDataPtr[] = { modFilterData };
        auto&& modBlock = dsp::AudioBlock<float> { modFilterDataPtr, 1, blockSize };
        modFilter.process (dsp::ProcessContextReplacing<float> { modBlock });

        for (int n = 0; n < blockSize; ++n)
            expectWithinAbsoluteError (modFilterData[n], originalFilterData[n], 1.0e-3f, "Sample " + String (n) + " is incorrect");
    }

    void firstOrderLPFTest()
    {
        using namespace Constants;
        using FilterType = chowdsp::FirstOrderLPF<double>;
        FilterType originalFilter;
        originalFilter.reset();
        originalFilter.calcCoefs ((double) cutoffFreq, (double) sampleRate);

        double originalFilterData[blockSize] {};
        for (int n = 0; n < blockSize; ++n)
            originalFilterData[n] = originalFilter.processSample ((double) data[n]);

        chowdsp::ModFilterWrapper<FilterType> modFilter;
        modFilter.prepare ({ sampleRate, (uint32) blockSize, 1 });
        modFilter.calcCoefs ((double) cutoffFreq, (double) sampleRate);

        double modFilterData[blockSize] {};
        std::copy (data, data + blockSize, modFilterData);

        double* modFilterDataPtr[] = { modFilterData };
        auto&& modBlock = dsp::AudioBlock<double> { modFilterDataPtr, 1, blockSize };
        modFilter.process (dsp::ProcessContextReplacing<double> { modBlock });

        for (int n = 0; n < blockSize; ++n)
            expectWithinAbsoluteError (modFilterData[n], originalFilterData[n], 1.0e-3, "Sample " + String (n) + " is incorrect");
    }

    void bypassTest()
    {
        using namespace Constants;
        using FilterType = chowdsp::PeakingFilter<float>;
        chowdsp::ModFilterWrapper<FilterType> modFilter;
        modFilter.prepare ({ sampleRate, (uint32) blockSize, 1 });
        modFilter.calcCoefs (cutoffFreq, filterQ, filterGain, sampleRate);

        float modFilterData[blockSize] {};
        std::copy (data, data + blockSize, modFilterData);

        float* modFilterDataPtr[] = { modFilterData };
        auto&& modBlock = dsp::AudioBlock<float> { modFilterDataPtr, 1, blockSize };
        auto&& context = dsp::ProcessContextReplacing<float> { modBlock };
        context.isBypassed = true;
        modFilter.process (context);

        for (int n = 0; n < blockSize; ++n)
            expectEquals (modFilterData[n], data[n], "Sample " + String (n) + " is incorrect");
    }

    void runTestTimed() override
    {
        beginTest ("Peaking Filter Test");
        peakingFilterTest();

        beginTest ("First-Order LPD Test");
        firstOrderLPFTest();

        beginTest ("Bypass Test");
        bypassTest();
    }
};

static ModFilterWrapperTest modFilterWrapperTest;
