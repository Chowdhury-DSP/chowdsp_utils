#include "chowdsp_AudioUIBackgroundTask.h"

namespace chowdsp
{
#ifndef DOXYGEN
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

    void TimeSliceBackgroundTask::setTimeSliceThreadToUse (juce::TimeSliceThread* newTimeSliceThreadToUse)
    {
        const auto wasRunning = isBackgroundTaskRunning();
        if (wasRunning)
            stopTask();

        timeSliceThreadToUse = newTimeSliceThreadToUse;

        if (wasRunning)
            startTask();
    }

    bool TimeSliceBackgroundTask::isBackgroundTaskRunning() const
    {
        for (int i = 0; i < timeSliceThreadToUse->getNumClients(); ++i)
        {
            if (timeSliceThreadToUse->getClient (i) == this)
                return true;
        }

        return false;
    }

    void TimeSliceBackgroundTask::startTask()
    {
        timeSliceThreadToUse->addTimeSliceClient (this);

        if (! timeSliceThreadToUse->isThreadRunning())
            timeSliceThreadToUse->startThread();
    }

    void TimeSliceBackgroundTask::stopTask()
    {
        timeSliceThreadToUse->removeTimeSliceClient (this);

        if (timeSliceThreadToUse->getNumClients() == 0)
            timeSliceThreadToUse->stopThread (-1);
    }
} // namespace detail
#endif // DOXYGEN

template <typename BackgroundTaskType>
AudioUIBackgroundTask<BackgroundTaskType>::AudioUIBackgroundTask (const juce::String& name) : BackgroundTaskType (name)
{
}

template <typename BackgroundTaskType>
AudioUIBackgroundTask<BackgroundTaskType>::~AudioUIBackgroundTask()
{
    if (this->isBackgroundTaskRunning())
    {
        jassertfalse; // You should always stop running the background task before trying to delete it!
        this->stopTask();
    }
}

template <typename BackgroundTaskType>
void AudioUIBackgroundTask<BackgroundTaskType>::prepare (double sampleRate, int samplesPerBlock, int numChannels)
{
    if (this->isBackgroundTaskRunning())
        this->stopTask();

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
        this->startTask();
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

    if (! shouldRun && this->isBackgroundTaskRunning())
    {
        this->stopTask();
        return;
    }

    if (isPrepared && shouldRun && ! this->isBackgroundTaskRunning())
    {
        this->startTask();
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
template class AudioUIBackgroundTask<detail::TimeSliceBackgroundTask>;
} // namespace chowdsp
