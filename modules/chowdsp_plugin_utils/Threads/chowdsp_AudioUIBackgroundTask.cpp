#include "chowdsp_AudioUIBackgroundTask.h"

namespace chowdsp
{
namespace detail
{
    void SingleThreadBackgroundTask::run()
    {
        while (true)
        {
            if (this->threadShouldExit())
                return;

            this->wait (runTaskOnBackgroundThread());
        }
    }

    TimeSliceBackgroundTask::TimeSliceBackgroundTask (const juce::String&)
    {
        timeSliceThreadToUse->addTimeSliceClient (this);
    }

    TimeSliceBackgroundTask::~TimeSliceBackgroundTask()
    {
        timeSliceThreadToUse->removeTimeSliceClient (this);
    }

    void TimeSliceBackgroundTask::setTimeSliceThreadToUse (juce::TimeSliceThread* newTimeSliceThreadToUse)
    {
        timeSliceThreadToUse->removeTimeSliceClient (this);
        timeSliceThreadToUse = newTimeSliceThreadToUse;
        timeSliceThreadToUse->addTimeSliceClient (this);
    }
} // namespace detail

template <typename BackgroundTaskType>
AudioUIBackgroundTask<BackgroundTaskType>::AudioUIBackgroundTask (const juce::String& name) : BackgroundTaskType (name)
{
}

template <typename BackgroundTaskType>
void AudioUIBackgroundTask<BackgroundTaskType>::prepare (double sampleRate, int samplesPerBlock, int numChannels)
{
    if (this->isThreadRunning())
        this->stopThread (-1);

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
        this->startThread();
}

template <typename BackgroundTaskType>
void AudioUIBackgroundTask<BackgroundTaskType>::reset()
{
    for (auto& buffer : data)
        buffer.clear();

    writePosition = 0;
    resetTask();
}

template <typename BackgroundTaskType>
void AudioUIBackgroundTask<BackgroundTaskType>::pushSamples (int channel, const float* samples, int numSamples)
{
    data[(size_t) channel].push (samples, numSamples);
    writePosition = data[(size_t) channel].getWritePointer();
}

template <typename BackgroundTaskType>
void AudioUIBackgroundTask<BackgroundTaskType>::pushSamples (const juce::AudioBuffer<float>& buffer)
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        data[(size_t) ch].push (buffer.getReadPointer (ch), buffer.getNumSamples());

    writePosition = data[0].getWritePointer();
}

template <typename BackgroundTaskType>
void AudioUIBackgroundTask<BackgroundTaskType>::setShouldBeRunning (bool shouldRun)
{
    shouldBeRunning = shouldRun;

    if (! shouldRun && this->isThreadRunning())
    {
        this->stopThread (-1);
        return;
    }

    if (isPrepared && shouldRun && ! this->isThreadRunning())
    {
        this->startThread();
        return;
    }
}

template <typename BackgroundTaskType>
int AudioUIBackgroundTask<BackgroundTaskType>::runTaskOnBackgroundThread()
{
    latestData.clear();
    const auto dataOffset = writePosition - requestedDataSize;
    for (int ch = 0; ch < latestData.getNumChannels(); ++ch)
        latestData.copyFrom (ch, 0, data[(size_t) ch].data (dataOffset), requestedDataSize);

    runTask (latestData);

    return waitMilliseconds;
}

template class AudioUIBackgroundTask<detail::SingleThreadBackgroundTask>;
//template class AudioUIBackgroundTask<juce::TimeSliceClient>;
} // namespace chowdsp
