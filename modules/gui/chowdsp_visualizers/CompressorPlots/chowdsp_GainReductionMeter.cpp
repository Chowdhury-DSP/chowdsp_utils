#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_utils && JUCE_MODULE_AVAILABLE_juce_dsp

#include "chowdsp_GainReductionMeter.h"

namespace chowdsp::compressor
{
constexpr int meterFps = 30; // @TUNE: select meter frame rate here

static int getYForDB (float dB, int height)
{
    constexpr auto maxDB = 1.0f;
    constexpr auto minDB = -31.0f;
    constexpr auto dBRange = maxDB - minDB;

    auto normLevel = juce::jmin (juce::jmax (dB - minDB, 0.0f) / dBRange, 1.0f);
    return int ((1.0f - normLevel) * (float) height);
}

void GainReductionMeter::BackgroundTask::prepareTask (double sampleRate, int samplesPerBlock, int& requestedBlockSize, int& waitMs)
{
    meterBuffer.setMaxSize (2, samplesPerBlock);

    ballisticsFilter.prepare ({ sampleRate, (juce::uint32) samplesPerBlock, 2 });

    // @TUNE: set ballistics filters parameters here
    ballisticsFilter.setAttackTime (10.0f);
    ballisticsFilter.setReleaseTime (500.0f);
    ballisticsFilter.setLevelCalculationType (juce::dsp::BallisticsFilterLevelCalculationType::peak);

    requestedBlockSize = int (sampleRate / (double) meterFps);
    waitMs = int (1000.0 / (double) meterFps);
}

void GainReductionMeter::BackgroundTask::runTask (const juce::AudioBuffer<float>& taskData)
{
    jassert (taskData.getNumChannels() == 2);
    const auto numSamples = taskData.getNumSamples();

    const auto* inputData = taskData.getReadPointer (0);
    const auto* compressedData = taskData.getReadPointer (1);

    float inputLevelPeak = 0.0f, compressedLevelPeak = 0.0f;
    for (int n = 0; n < numSamples; ++n)
    {
        inputLevelPeak = ballisticsFilter.processSample (0, std::abs (inputData[n]));
        compressedLevelPeak = ballisticsFilter.processSample (1, std::abs (compressedData[n]));
    }

    gainReductionDB = juce::approximatelyEqual (inputLevelPeak, 0.0f) ? 0.0f : juce::Decibels::gainToDecibels (compressedLevelPeak / inputLevelPeak);
}

void GainReductionMeter::BackgroundTask::pushBufferData (const chowdsp::BufferView<const float>& buffer, bool isInput)
{
    const auto inNumChannels = buffer.getNumChannels();
    const auto inNumSamples = buffer.getNumSamples();
    const auto destChannel = isInput ? 0 : 1;

    if (isInput)
        meterBuffer.setCurrentSize (2, inNumSamples);

    chowdsp::BufferMath::copyBufferChannels (buffer, meterBuffer, 0, destChannel);
    for (int ch = 1; ch < inNumChannels; ++ch)
        chowdsp::BufferMath::addBufferChannels (buffer, meterBuffer, ch, destChannel);
    juce::FloatVectorOperations::multiply (meterBuffer.getWritePointer (destChannel), 1.0f / (float) inNumChannels, inNumSamples);

    if (! isInput)
        pushSamples (meterBuffer.toAudioBuffer());
}

float GainReductionMeter::BackgroundTask::getGainReductionDB() const
{
    return gainReductionDB.load();
}

//===========================================================
GainReductionMeter::GainReductionMeter (BackgroundTask& backgroundTask) : task (backgroundTask)
{
    gainReductionSmoother.reset ((double) meterFps, 0.05); // @TUNE: set graphics smoothing time here
    gainReductionSmoother.setCurrentAndTargetValue (0.0f);

    task.setShouldBeRunning (true);
    startTimerHz (meterFps);
}

GainReductionMeter::~GainReductionMeter()
{
    task.setShouldBeRunning (false);
}

void GainReductionMeter::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);

    constexpr auto cornerSize = 5.0f;
    const auto meterWidth = proportionOfWidth (0.33f);
    const auto meterXpad = proportionOfWidth (0.14f);

    auto b = getLocalBounds();
    b.removeFromLeft (meterXpad);

    // draw meter background
    const auto width = b.getWidth();
    const auto height = b.getHeight();
    g.setColour (juce::Colours::black);
    auto meterRect = juce::Rectangle { (float) meterXpad, 0.0f, (float) meterWidth, (float) height };
    g.fillRoundedRectangle (meterRect, cornerSize);

    // draw dB labels
    g.setColour (juce::Colours::white);
    const auto dbLabelHeight = 0.03f * (float) getHeight();
    g.setFont (dbLabelHeight);
    for (auto dbLevel : { -30.0f, -25.0f, -20.0f, -15.0f, -10.0f, -5.0f, 0.0f })
    {
        auto dbY = getYForDB (dbLevel, height);
        auto dbRect = juce::Rectangle { width, (int) dbLabelHeight + 1 }
                          .withCentre (juce::Point (width / 2, dbY - 1))
                          .withLeft (meterWidth * 7 / 4);

        auto dbString = juce::String (dbLevel, 0);
        if (juce::approximatelyEqual (dbLevel, 0.0f))
            dbString += " dB";

        g.drawFittedText (dbString, dbRect, juce::Justification::centredLeft, 1);
    }

    // compute gain reduction things, and draw main gain reduction area
    gainReductionSmoother.setTargetValue (task.getGainReductionDB());
    g.setColour (juce::Colours::red);
    auto gainRedTop = (float) getYForDB (0.0f, height);
    auto gainRedBottom = (float) getYForDB (gainReductionSmoother.getNextValue(), height);
    auto gainRedRect = juce::Rectangle { (float) meterXpad, gainRedTop, (float) meterWidth, gainRedBottom - gainRedTop };
    g.fillRect (gainRedRect);
}
} // namespace chowdsp::compressor

#endif // JUCE_MODULE_AVAILABLE_chowdsp_plugin_utils && JUCE_MODULE_AVAILABLE_juce_dsp
