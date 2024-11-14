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
    static constexpr double rampLength = 0.000167;
    static constexpr std::array<float, blockSize> dbsToTest { -30.0f, -20.0f, -15.0f, -12.0f, -9.0f, -6.0f, 0.0f, 6.0f };
    static constexpr std::array<float, blockSize> autoMakeupTest { 4.0f, -3.0f, 4.0f, -3.0f, 4.0f, -3.0f, 4.0f, -3.0f };
    static constexpr std::array<float, blockSize> autoMakeupTestVariable { -2.875f, -2.75f, -2.625f, -2.5f, -2.375f, -2.25f, -2.125f, -2.0f };
    static constexpr std::array<float, blockSize> autoMakeupTestFeedBackFixed {
        -6.0f,
        4.0f,
        -6.0f,
        4.0f,
        -6.0f,
        4.0f,
        -6.0f,
        4.0f,
    };
    static constexpr std::array<float, blockSize> autoMakeupTestFeedBackVariable { -5.75f, -5.5f, -5.25f, -5.0f, -4.75f, -4.49f, -4.24f, -4.0f };
    static constexpr std::array<float, blockSize> autoMakeupTestCompressed { 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f, 16.0f };

    chowdsp::SmoothedBufferValue<float, juce::ValueSmoothingTypes::Multiplicative> threshSmoothFixed;
    chowdsp::SmoothedBufferValue<float, juce::ValueSmoothingTypes::Multiplicative> threshSmoothCompressed;
    chowdsp::SmoothedBufferValue<float, juce::ValueSmoothingTypes::Multiplicative> threshSmoothVariable;
    chowdsp::SmoothedBufferValue<float, juce::ValueSmoothingTypes::Multiplicative> ratioSmooth;

    //prepare and process thresh smooth
    threshSmoothFixed.mappingFunction = [] (float x)
    { return juce::Decibels::decibelsToGain (x); };
    threshSmoothFixed.prepare (fs, blockSize);
    threshSmoothFixed.reset (-6.0f);
    threshSmoothFixed.process (threshSmoothFixed.getCurrentValue(), blockSize);

    threshSmoothVariable.mappingFunction = [] (float x)
    { return juce::Decibels::decibelsToGain (x); };
    threshSmoothVariable.prepare (fs, blockSize);
    threshSmoothVariable.setRampLength (rampLength);
    threshSmoothVariable.reset (-6.0f);
    threshSmoothVariable.process (-4.0, blockSize);

    threshSmoothCompressed.mappingFunction = [] (float x)
    { return juce::Decibels::decibelsToGain (x); };
    threshSmoothCompressed.prepare (fs, blockSize);
    threshSmoothCompressed.reset (-3.0f);
    threshSmoothCompressed.process (threshSmoothCompressed.getCurrentValue(), blockSize);

    //prepare and process ratio smooth
    ratioSmooth.prepare (fs, blockSize);
    ratioSmooth.reset (2.0f);
    ratioSmooth.process (ratioSmooth.getCurrentValue(), blockSize);

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

    chowdsp::compressor::GainComputerParams<float> gainComputerParamsCompressed {
        threshSmoothCompressed,
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
            gainSample = juce::Decibels::decibelsToGain (dbBuffer[n]);
    };

    //convert input buffers to linear gain
    chowdsp::StaticBuffer<float, 1, blockSize> inBuffer { 1, blockSize };
    dBToGain (dbsToTest, inBuffer);

    chowdsp::StaticBuffer<float, 1, blockSize> autoMakeupTestBuffer { 1, blockSize };
    dBToGain (autoMakeupTest, autoMakeupTestBuffer);

    chowdsp::StaticBuffer<float, 1, blockSize> autoMakeupTestVariableBuffer { 1, blockSize };
    dBToGain (autoMakeupTestVariable, autoMakeupTestVariableBuffer);

    chowdsp::StaticBuffer<float, 1, blockSize> autoMakeupTestFeedBackFixedBuffer { 1, blockSize };
    dBToGain (autoMakeupTestFeedBackFixed, autoMakeupTestFeedBackFixedBuffer);

    chowdsp::StaticBuffer<float, 1, blockSize> autoMakeupTestFeedBackVariableBuffer { 1, blockSize };
    dBToGain (autoMakeupTestFeedBackVariable, autoMakeupTestFeedBackVariableBuffer);

    chowdsp::StaticBuffer<float, 1, blockSize> autoMakeupTestCompressedBuffer { 1, blockSize };
    dBToGain (autoMakeupTestCompressed, autoMakeupTestCompressedBuffer);

    gainComputer.setThreshold (-12.0f);
    gainComputer.setRatio (4.0f);
    gainComputer.setKnee (6.0f);
    gainComputer.reset();

    chowdsp::StaticBuffer<float, 1, blockSize> outBuffer { 1, blockSize };

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

    SECTION ("Feed-Forward Apply Auto Makeup w/ Fixed Threshold and Ratio")
    {
        chowdsp::compressor::FeedForwardCompGainComputer<float> feedForwardCompGainComputerFixed {};
        feedForwardCompGainComputerFixed.applyAutoMakeup (autoMakeupTestBuffer, gainComputerParamsFixed);
        checkData ({ 7.0f, 0.0f, 7.0f, 0.0f, 7.0f, 0.0f, 7.0f, 0.0f }, autoMakeupTestBuffer);
    }

    SECTION ("Feed-Forward Apply Auto Makeup w/ Variable Threshold and Ratio")
    {
        chowdsp::compressor::FeedForwardCompGainComputer<float> feedForwardCompGainComputerVariable {};
        feedForwardCompGainComputerVariable.applyAutoMakeup (autoMakeupTestVariableBuffer, gainComputerParamsVariable);
        checkData ({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, autoMakeupTestVariableBuffer);
    }

    SECTION ("Feed-Back Apply Auto Makeup w/ Fixed Threshold and Ratio")
    {
        chowdsp::compressor::FeedBackCompGainComputer<float> feedBackCompGainComputerFixed {};
        feedBackCompGainComputerFixed.applyAutoMakeup (autoMakeupTestFeedBackFixedBuffer, gainComputerParamsFixed);
        checkData ({ 0.0f, 10.0f, 0.0f, 10.0f, 0.0f, 10.0f, 0.0f, 10.0f }, autoMakeupTestFeedBackFixedBuffer);
    }

    SECTION ("Feed-Back Apply Auto Makeup w/ Variable Threshold and Ratio")
    {
        chowdsp::compressor::FeedBackCompGainComputer<float> feedBackCompGainComputerVariable {};
        feedBackCompGainComputerVariable.applyAutoMakeup (autoMakeupTestFeedBackVariableBuffer, gainComputerParamsVariable);
        checkData ({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, autoMakeupTestFeedBackVariableBuffer);
    }

    SECTION ("Feed-Back Apply Compression and Auto Makeup w/ Fixed Threshold and Ratio")
    {
        chowdsp::compressor::FeedForwardCompGainComputer<float> feedForwardGainComputerCompressed {};
        feedForwardGainComputerCompressed.process (autoMakeupTestCompressedBuffer, outBuffer, gainComputerParamsCompressed);
        for (auto [n, outSample] : chowdsp::enumerate (autoMakeupTestCompressedBuffer.getWriteSpan (0)))
            outSample *= outBuffer.getReadSpan (0)[n];
        checkData ({ -9.5f, -9.5f, -9.5f, -9.5f, -9.5f, -9.5f, -9.5f, -9.5f }, outBuffer); // gain reduction buffer
        checkData ({ 6.5f, 6.5f, 6.5f, 6.5f, 6.5f, 6.5f, 6.5f, 6.5f }, autoMakeupTestCompressedBuffer); //compressed signal
        feedForwardGainComputerCompressed.applyAutoMakeup (autoMakeupTestCompressedBuffer, gainComputerParamsCompressed);
        checkData ({ 8.0f, 8.0f, 8.0f, 8.0f, 8.0f, 8.0f, 8.0f, 8.0f }, autoMakeupTestCompressedBuffer); //compressed signal with makeup gain
    }
}