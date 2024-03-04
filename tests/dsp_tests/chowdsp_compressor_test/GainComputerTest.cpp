#include <CatchUtils.h>
#include <iostream>
#include <iomanip>
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
    static constexpr std::array<float, blockSize> autoMakeupTest { 4.0f, -3.0103f, 4.0f, -3.0103f, 4.0f, -3.0103f, 4.0f, -3.0103f };
    static constexpr std::array<float, blockSize> autoMakeupTestVariable { -2.875f ,-2.75f, -2.625f, -2.5f, -2.375f, -2.25f, -2.125f, -2.0f };

    chowdsp::SmoothedBufferValue<float, juce::ValueSmoothingTypes::Multiplicative> threshSmoothFixed;
    chowdsp::SmoothedBufferValue<float, juce::ValueSmoothingTypes::Multiplicative> threshSmoothVariable;
    chowdsp::SmoothedBufferValue<float, juce::ValueSmoothingTypes::Multiplicative> ratioSmooth;

    //prepare and process thresh smooth
    threshSmoothFixed.mappingFunction = [] (float x) { return juce::Decibels::decibelsToGain (x); };
    threshSmoothFixed.prepare(fs, blockSize);
    threshSmoothFixed.reset(-6.0f);

    threshSmoothVariable.mappingFunction = [] (float x) { return juce::Decibels::decibelsToGain (x); };
    threshSmoothVariable.prepare(fs, blockSize);
    threshSmoothVariable.setRampLength(0.000167);
    threshSmoothVariable.reset(-6.0f);
    threshSmoothVariable.process(-4.0, blockSize);


    //prepare and process ratio smooth
    ratioSmooth.prepare(fs, blockSize);
    ratioSmooth.reset(2.0f);

    chowdsp::compressor::GainComputerParams<float> gainComputerParamsFixed {
        threshSmoothFixed,
        ratioSmooth,
        0.0f,
        0.0f,
        0.0f
    };

    chowdsp::compressor::GainComputerParams<float> gainComputerParamsVariable {
        threshSmoothVariable,
        ratioSmooth,
        0.0f,
        0.0f,
        0.0f
    };

    GainComputer gainComputer;
    gainComputer.prepare (fs, blockSize);

    auto dBToGain = [&] (const std::array<float, blockSize>& dbBuffer, chowdsp::StaticBuffer<float, 1, blockSize>& gainBuffer)
    {
        for (auto [n, gainSample] : chowdsp::enumerate (gainBuffer.getWriteSpan (0)))
            gainSample = juce::Decibels::decibelsToGain(dbBuffer[n]);
    };

    chowdsp::StaticBuffer<float, 1, blockSize> inBuffer { 1, blockSize };
    dBToGain (dbsToTest, inBuffer);

    chowdsp::StaticBuffer<float, 1, blockSize> autoMakeupTestBuffer { 1, blockSize };
    dBToGain (autoMakeupTest, autoMakeupTestBuffer);

    chowdsp::StaticBuffer<float, 1, blockSize> autoMakeupTestVariableBuffer { 1, blockSize };
    dBToGain (autoMakeupTestVariable, autoMakeupTestVariableBuffer);

    gainComputer.setThreshold (-12.0f);
    gainComputer.setRatio (4.0f);
    gainComputer.setKnee (6.0f);
    gainComputer.reset();

    chowdsp::StaticBuffer<float, 1, blockSize> outBuffer { 1, blockSize };
    chowdsp::StaticBuffer<float, 1, blockSize> inverseGainBuffer { 1, blockSize };

    const auto checkData = [&] (const std::array<float, blockSize>& dbExpected, chowdsp::StaticBuffer<float, 1, blockSize>& buffer)
    {
        for (auto [n, sample] : chowdsp::enumerate (buffer.getReadSpan (0)))
        {
            const auto dbActual = juce::Decibels::gainToDecibels (sample);
            REQUIRE (dbActual == Catch::Approx { dbExpected[n] }.margin (0.1f));
        }
    };

    SECTION ("Feed-Forward")
    {
        gainComputer.setMode (0);
        gainComputer.processBlock (inBuffer, outBuffer);
        checkData ({ 0.0f, 0.0f, 0.0f, -0.5f, -2.25f, -4.5f, -9.0f, -13.5f }, outBuffer);
    }

    SECTION ("Feed-Forward w/ Makeup")
    {
        gainComputer.setMode (0);
        gainComputer.processBlock (inBuffer, outBuffer);
        gainComputer.applyAutoMakeup (outBuffer);
        checkData ({ 9.0f, 9.0f, 9.0f, 8.5f, 6.75f, 4.5f, 0.0f, -4.5f }, outBuffer);
    }

    SECTION ("Feed-Back")
    {
        gainComputer.setMode (1);
        gainComputer.processBlock (inBuffer, outBuffer);
        checkData ({ 0.0f, 0.0f, 0.0f, -2.25f, -9.0f, -18.0f, -36.0f, -54.0f }, outBuffer);
    }

    SECTION ("Feed-Back w/ Makeup")
    {
        gainComputer.setMode (1);
        gainComputer.processBlock (inBuffer, outBuffer);
        gainComputer.applyAutoMakeup (outBuffer);
        checkData ({ 36.0f, 36.0f, 36.0f, 33.75f, 27.0f, 18.0f, 0.0f, -18.0f }, outBuffer);
    }

    SECTION("feed-forward apply auto make up with fixed threshold and ratio")
    {
        chowdsp::compressor::FeedForwardCompGainComputer<float> feedForwardCompGainComputerFixed{};
        feedForwardCompGainComputerFixed.applyAutoMakeup(autoMakeupTestBuffer, gainComputerParamsFixed);
        checkData ({ 7.0103f, 0.0f, 7.0103f, 0.0f, 7.0103f, 0.0f, 7.0103f, 0.0f}, autoMakeupTestBuffer);
    }

    SECTION("feed-forward apply auto make up with variable threshold and fixed ratio")
    {
        chowdsp::compressor::FeedForwardCompGainComputer<float> feedForwardCompGainComputerVariable{};
        feedForwardCompGainComputerVariable.applyAutoMakeup(autoMakeupTestVariableBuffer, gainComputerParamsVariable);
        for (auto [n, makeupSample] : chowdsp::enumerate (autoMakeupTestVariableBuffer.getReadSpan (0)))
            std::cout << juce::Decibels::gainToDecibels(makeupSample) << " ";
        checkData ({0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}, autoMakeupTestVariableBuffer);
    }
}