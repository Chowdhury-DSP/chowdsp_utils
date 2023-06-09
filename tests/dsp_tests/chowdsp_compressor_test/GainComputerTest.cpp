#include <CatchUtils.h>
#include <chowdsp_compressor/chowdsp_compressor.h>

namespace chow_comp = chowdsp::compressor;
using GainComputer = chow_comp::GainComputer<float,
                                             types_list::TypesList<chow_comp::FeedForwardCompGainComputer<float>,
                                                                   chow_comp::FeedBackCompGainComputer<float>>>;

TEST_CASE ("Gain Computer Test", "[dsp][compressor]")
{
    static constexpr double fs = 48000.0;
    static constexpr int blockSize = 8;
    static constexpr std::array<float, blockSize> dbsToTest { -30.0f, -20.0f, -15.0f, -12.0f, -9.0f, -6.0f, 0.0f, 6.0f };

    GainComputer gainComputer;
    gainComputer.prepare (fs, blockSize);

    chowdsp::StaticBuffer<float, 1, blockSize> inBuffer { 1, blockSize };
    for (auto [n, sample] : chowdsp::enumerate (inBuffer.getWriteSpan (0)))
        sample = juce::Decibels::decibelsToGain (dbsToTest[n]);

    gainComputer.setThreshold (-12.0f);
    gainComputer.setRatio (4.0f);
    gainComputer.setKnee (6.0f);
    gainComputer.reset();

    chowdsp::StaticBuffer<float, 1, blockSize> outBuffer { 1, blockSize };

    const auto checkData = [&outBuffer] (const std::array<float, blockSize>& dbExpected)
    {
        for (auto [n, sample] : chowdsp::enumerate (outBuffer.getReadSpan (0)))
        {
            const auto dbActual = juce::Decibels::gainToDecibels (sample);
            REQUIRE (dbActual == Catch::Approx { dbExpected[n] }.margin (0.1f));
        }
    };

    SECTION ("Feed-Forward")
    {
        gainComputer.setMode (0);
        gainComputer.processBlock (inBuffer, outBuffer);
        checkData ({ 0.0f, 0.0f, 0.0f, -0.5f, -2.25f, -4.5f, -9.0f, -13.5f });
    }

    SECTION ("Feed-Forward w/ Makeup")
    {
        gainComputer.setMode (0);
        gainComputer.processBlock (inBuffer, outBuffer);
        gainComputer.applyAutoMakeup (outBuffer);
        checkData ({ 9.0f, 9.0f, 9.0f, 8.5f, 6.75f, 4.5f, 0.0f, -4.5f });
    }

    SECTION ("Feed-Back")
    {
        gainComputer.setMode (1);
        gainComputer.processBlock (inBuffer, outBuffer);
        checkData ({ 0.0f, 0.0f, 0.0f, -2.25f, -9.0f, -18.0f, -36.0f, -54.0f });
    }

    SECTION ("Feed-Back w/ Makeup")
    {
        gainComputer.setMode (1);
        gainComputer.processBlock (inBuffer, outBuffer);
        gainComputer.applyAutoMakeup (outBuffer);
        checkData ({ 36.0f, 36.0f, 36.0f, 33.75f, 27.0f, 18.0f, 0.0f, -18.0f });
    }
}
