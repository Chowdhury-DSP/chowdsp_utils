#include <CatchUtils.h>
#include <iostream>
#include <chowdsp_compressor/chowdsp_compressor.h>

namespace chow_comp = chowdsp::compressor;
using GainComputer = chow_comp::GainComputer<float,
                                             types_list::TypesList<chow_comp::FeedForwardCompGainComputer<float>,
                                                                   chow_comp::FeedBackCompGainComputer<float>>>;

TEST_CASE ("Gain Computer Test", "[dsp][compressor]")
{
    static constexpr double fs = 48000.0;
    static constexpr int blockSize = 8;
    static constexpr double rampLength = 0.125;
    static constexpr std::array<float, blockSize> dbsToTest { -30.0f, -20.0f, -15.0f, -12.0f, -9.0f, -6.0f, 0.0f, 6.0f };

    chowdsp::SmoothedBufferValue<float, juce::ValueSmoothingTypes::Multiplicative> threshSmooth;
    chowdsp::SmoothedBufferValue<float, juce::ValueSmoothingTypes::Multiplicative> ratioSmooth;
    //prepare and process thresh smooth
    threshSmooth.mappingFunction = [] (float x) { return juce::Decibels::decibelsToGain (x); };
    threshSmooth.prepare(fs, blockSize);
    threshSmooth.setRampLength(rampLength);
    threshSmooth.reset(-12.0f);
    threshSmooth.process(-18.0f, blockSize);

    //prepare and process ratio smooth
    ratioSmooth.prepare(fs, blockSize);
    ratioSmooth.setRampLength(rampLength);
    ratioSmooth.reset(2.0f);
    ratioSmooth.process(4.0f, blockSize);

    chowdsp::compressor::GainComputerParams<float> gainComputerParams {
        threshSmooth,
        ratioSmooth,
        6.0f,
        -12.0f,
        -18.0f
    };

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
    chowdsp::StaticBuffer<float, 1, blockSize> gainBuffer { 1, blockSize };

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

    SECTION ("Feed-Forward Comp Gain Computer Apply Auto Makeup")
    {
        chowdsp::compressor::FeedForwardCompGainComputer<float> feedForwardCompGainComputer{};
        //calculate gain adjustment based on the level of the input buffer
        feedForwardCompGainComputer.process(inBuffer, gainBuffer, gainComputerParams);
        //multiply input buffer by gain adjustment buffer
        for (auto [n, sampleCompressed] : chowdsp::enumerate (inBuffer.getWriteSpan(0)))
        {
            sampleCompressed *= gainBuffer.getReadSpan (0)[n];
            std::cout << sampleCompressed << " ";
        }
        //Apply auto makeup to the compressed buffer
        feedForwardCompGainComputer.applyAutoMakeup(inBuffer, gainComputerParams);
        //verify the correctness of applyAutoMakeup
    }

//    SECTION ("Feed-Back Camp Gain Computer w/ Makeup")
//    {
//        chowdsp::compressor::FeedBackCompGainComputer<float> feedBackCompGainComputer{};
//        feedBackCompGainComputer.process(inBuffer, gainBuffer, gainComputerParams);
//        feedBackCompGainComputer.applyAutoMakeup(inBuffer, gainComputerParams);
//        //checkData(//expected dBs)
//    }
}