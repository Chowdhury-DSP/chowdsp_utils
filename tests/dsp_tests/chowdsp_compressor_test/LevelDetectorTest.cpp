#include <CatchUtils.h>
#include <chowdsp_compressor/chowdsp_compressor.h>

namespace chow_comp = chowdsp::compressor;
using LevelDetector = chow_comp::CompressorLevelDetector<float,
                                                         types_list::TypesList<chow_comp::PeakDetector,
                                                                               chow_comp::RMSDetector,
                                                                               chow_comp::PeakRtTDetector>>;

TEST_CASE ("Level Detector Test", "[dsp][compressor]")
{
    const auto msToSamples = [] (double ms, double fs)
    {
        return int ((ms * 0.001) * fs);
    };

    static constexpr double fs = 48000.0;
    static constexpr int blockSize = msToSamples (500.0, fs);
    static constexpr std::array<int, 5> samplesToCheck { msToSamples (25.0, fs),
                                                         msToSamples (55.0, fs),
                                                         msToSamples (125.0, fs),
                                                         msToSamples (175.0, fs),
                                                         msToSamples (495.0, fs) };

    chowdsp::StaticBuffer<float, 1, blockSize> buffer { 1, blockSize };
    std::fill (buffer.getWriteSpan (0).begin() + msToSamples (50.0, fs),
               buffer.getWriteSpan (0).begin() + msToSamples (150.0, fs),
               1.0f);

    const auto checkBuffer = [&buffer, sampleIndexes = samplesToCheck] (const std::array<float, 5>& expectedLevels)
    {
        for (auto [n, sampleIndex] : chowdsp::enumerate (sampleIndexes))
            REQUIRE (buffer.getReadSpan (0)[(size_t) sampleIndex] == Catch::Approx { expectedLevels[n] }.margin (0.05f));
    };

    LevelDetector levelDetector;
    levelDetector.prepare (fs, blockSize);
    levelDetector.setAttackMs (10.0f);
    levelDetector.setReleaseMs (50.0f);

    SECTION ("Peak Detector")
    {
        levelDetector.setMode (0);
        levelDetector.processBlock (buffer);
        checkBuffer ({ 0.0f, 0.39f, 1.0f, 0.606f, 0.0f });
    }

    SECTION ("RMS Detector")
    {
        levelDetector.setMode (1);
        levelDetector.processBlock (buffer);
        checkBuffer ({ 0.0f, 0.63f, 1.0f, 0.78f, 0.0f });
    }

    SECTION ("Peak Return-to-Threshold Detector")
    {
        levelDetector.setMode (2);
        levelDetector.setThresholdDB (-6.0f);
        levelDetector.processBlock (buffer);
        checkBuffer ({ 0.19f, 0.59f, 1.0f, 0.80f, 0.5f });
    }
}
