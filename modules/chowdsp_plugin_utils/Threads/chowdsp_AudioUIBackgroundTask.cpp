#include "chowdsp_AudioUIBackgroundTask.h"

namespace chowdsp
{
AudioUIBackgroundTask::AudioUIBackgroundTask (const juce::String& name) : juce::Thread (name)
{
}

void AudioUIBackgroundTask::prepare (double sampleRate, int samplesPerBlock, int numChannels)
{
    if (isThreadRunning())
        stopThread (-1);

    isPrepared = false;

    waitMilliseconds = -1;
    prepareTask (sampleRate, samplesPerBlock, requestedDataSize, waitMilliseconds);

    data.clear();
    const auto dataSize = 2 * juce::jmax (requestedDataSize, samplesPerBlock);
    for (int ch = 0; ch < numChannels; ++ch)
        data.emplace_back (dataSize);

    latestData.setSize (numChannels, requestedDataSize);

    if (waitMilliseconds < 0)
    {
        auto refreshTime = (double) data[0].size() / sampleRate; // time (seconds) for the whole buffer to be refreshed
        waitMilliseconds = int (1000.0 * refreshTime);
    }

    writePosition = 0;
    isPrepared = true;

    if (shouldBeRunning)
        startThread();
}

void AudioUIBackgroundTask::reset()
{
    for (auto& buffer : data)
        buffer.clear();

    writePosition = 0;
    resetTask();
}

void AudioUIBackgroundTask::pushSamples (int channel, const float* samples, int numSamples)
{
    data[(size_t) channel].push (samples, numSamples);
    writePosition = data[(size_t) channel].getWritePointer();
}

void AudioUIBackgroundTask::pushSamples (const juce::AudioBuffer<float>& buffer)
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        data[(size_t) ch].push (buffer.getReadPointer (ch), buffer.getNumSamples());

    writePosition = data[0].getWritePointer();
}

void AudioUIBackgroundTask::setShouldBeRunning (bool shouldRun)
{
    shouldBeRunning = shouldRun;

    if (! shouldRun && isThreadRunning())
    {
        stopThread (-1);
        return;
    }

    if (isPrepared && shouldRun && ! isThreadRunning())
    {
        startThread();
        return;
    }
}

void AudioUIBackgroundTask::run()
{
    while (true)
    {
        if (threadShouldExit())
            return;

        latestData.clear();
        const auto dataOffset = writePosition - requestedDataSize;
        for (int ch = 0; ch < latestData.getNumChannels(); ++ch)
            latestData.copyFrom (ch, 0, data[(size_t) ch].data (dataOffset), requestedDataSize);

        runTask (latestData);

        wait (waitMilliseconds);
    }
}
} // namespace chowdsp
