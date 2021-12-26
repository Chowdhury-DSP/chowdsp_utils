#include "chowdsp_AudioUIBackgroundTask.h"

namespace chowdsp
{

AudioUIBackgroundTask::AudioUIBackgroundTask (const juce::String& name) : juce::Thread (name)
{
}

void AudioUIBackgroundTask::prepare (double sampleRate, int samplesPerBlock)
{
    if (isThreadRunning())
        stopThread (-1);

    isPrepared = false;

    prepareTask (sampleRate, samplesPerBlock, requestedDataSize);
    data.resize (2 * juce::jmax (requestedDataSize, samplesPerBlock));

    auto refreshTime = (double) data.size() / sampleRate; // time (seconds) for the whole buffer to be refreshed
    waitMilliseconds = int (1000.0 * refreshTime);

    writePosition = 0;
    isPrepared = true;

    if (shouldBeRunning)
        startThread();
}

void AudioUIBackgroundTask::pushSamples (const float* samples, int numSamples)
{
    data.push (samples, numSamples);
    writePosition = data.getWritePointer();
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

        const auto* latestData = data.data (writePosition - requestedDataSize);
        runTask (latestData);

        wait (waitMilliseconds);
    }
}
} // namespace chowdsp
