#include <CatchUtils.h>
#include <chowdsp_filters/chowdsp_filters.h>

namespace Constants
{
constexpr float sampleRate = 48000.0f;
constexpr int blockSize = 8;
constexpr float cutoffFreq = 200.0f;
constexpr float filterQ = 0.7071f;
constexpr float filterGain = 2.0f;
constexpr float data[blockSize] = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
} // namespace Constants

TEST_CASE ("Mod Filter Wrapper Test", "[dsp][filters]")
{
    SECTION ("Peaking Filter Test")
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
        modFilter.prepare ({ sampleRate, (juce::uint32) blockSize, 1 });
        modFilter.calcCoefs (cutoffFreq, filterQ, filterGain, sampleRate);

        float modFilterData[blockSize] {};
        std::copy (data, data + blockSize, modFilterData);

        auto&& modBlock = chowdsp::BufferView { modFilterData, blockSize };
        modFilter.processBlock (modBlock);

        for (int n = 0; n < blockSize; ++n)
            REQUIRE_MESSAGE (modFilterData[n] == Catch::Approx (originalFilterData[n]).margin (1.0e-3f), "Sample " << std::to_string (n) << " is incorrect");
    }

    SECTION ("First-Order LPF Test")
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
        modFilter.prepare ({ sampleRate, (juce::uint32) blockSize, 1 });
        modFilter.calcCoefs ((double) cutoffFreq, (double) sampleRate);

        double modFilterData[blockSize] {};
        std::copy (data, data + blockSize, modFilterData);

        auto&& modBlock = chowdsp::BufferView { modFilterData, blockSize };
        modFilter.processBlock (modBlock);

        for (int n = 0; n < blockSize; ++n)
            REQUIRE_MESSAGE (modFilterData[n] == Catch::Approx (originalFilterData[n]).margin (1.0e-3f), "Sample " << std::to_string (n) << " is incorrect");
    }

    // @TODO: test this with juce_dsp
    //    SECTION ("Bypass Test")
    //    {
    //        using namespace Constants;
    //        using FilterType = chowdsp::PeakingFilter<float>;
    //        chowdsp::ModFilterWrapper<FilterType> modFilter;
    //        modFilter.prepare ({ sampleRate, (juce::uint32) blockSize, 1 });
    //        modFilter.calcCoefs (cutoffFreq, filterQ, filterGain, sampleRate);
    //
    //        float modFilterData[blockSize] {};
    //        std::copy (data, data + blockSize, modFilterData);
    //
    //        float* modFilterDataPtr[] = { modFilterData };
    //        auto&& modBlock = juce::dsp::AudioBlock<float> { modFilterDataPtr, 1, blockSize };
    //        auto&& context = juce::dsp::ProcessContextReplacing<float> { modBlock };
    //        context.isBypassed = true;
    //        modFilter.process (context);
    //
    //        for (int n = 0; n < blockSize; ++n)
    //            expectEquals (modFilterData[n], data[n], "Sample " + juce::String (n) + " is incorrect");
    //    }
}
