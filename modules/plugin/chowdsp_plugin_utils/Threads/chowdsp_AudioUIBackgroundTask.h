#pragma once

namespace chowdsp
{
#ifndef DOXYGEN
namespace detail
{
    /** Simple wrapper around juce::Thread that is compatible with AudioUIBackgroundTask */
    struct SingleThreadBackgroundTask : private juce::Thread
    {
        explicit SingleThreadBackgroundTask (const juce::String& name) : juce::Thread (name) {}

        void run() override;
        virtual int runTaskOnBackgroundThread() = 0;

        [[nodiscard]] bool isBackgroundTaskRunning() const { return isThreadRunning(); }
        void startTask() { startThread(); }
        void stopTask() { stopThread (-1); }
    };

    /** juce::TimeSliceClient that is compatible with AudioUIBackgroundTask */
    struct TimeSliceBackgroundTask : private juce::TimeSliceClient
    {
        explicit TimeSliceBackgroundTask (const juce::String&) {}

        void setTimeSliceThreadToUse (juce::TimeSliceThread* newTimeSliceThreadToUse);

        int useTimeSlice() override { return runTaskOnBackgroundThread(); }
        virtual int runTaskOnBackgroundThread() = 0;

        [[nodiscard]] bool isBackgroundTaskRunning() const;
        void startTask();
        void stopTask();

        struct TimeSliceThread : juce::TimeSliceThread
        {
            TimeSliceThread() : juce::TimeSliceThread ("Audio UI Background Thread") {}
        };

    private:
        juce::SharedResourcePointer<TimeSliceThread> sharedTimeSliceThread;
        juce::TimeSliceThread* timeSliceThreadToUse = sharedTimeSliceThread;
    };
} // namespace detail
#endif // DOXYGEN

/**
 * Let's say you need a class that can accept data from the audio thread,
 * do some computation with that data on a background thread, and then report
 * the computation result to the UI thread. This would be that class!
 *
 * The common scenario here is when you need a meter, or other audio visualization.
 *
 * It is recommended to use a type alias, like `SingleThreadAudioUIBackgroundTask`
 * or `TimeSliceAudioUIBackgroundTask` instead of using this class directly.
 */
template <typename BackgroundTaskType>
class AudioUIBackgroundTask : private BackgroundTaskType
{
public:
    /** Constructor with a name for the background thread */
    explicit AudioUIBackgroundTask (const juce::String& name);

    /** Default destructor */
    ~AudioUIBackgroundTask() override;

    /** Prepares the class to accept a new audio stream */
    void prepare (double sampleRate, int samplesPerBlock, int numChannels);

    /** Reset's the task state */
    void reset();

    /** Call this from the audio thread to push a new block of samples */
    void pushSamples (const juce::AudioBuffer<float>& buffer);

    /** Call this from the audio thread to push a new block of samples */
    void pushSamples (int channel, const float* samples, int numSamples);

    /** Set this method from the UI thread when you want the background task to start/stop running */
    void setShouldBeRunning (bool shouldRun);

    /** Returns true if the task is currently running */
    [[nodiscard]] bool isTaskRunning() const { return this->isBackgroundTaskRunning(); }

    /** Assigns this task to use a custom TimeSliceThread, rather than the default shared TimeSliceThread */
    template <typename Type = BackgroundTaskType>
    typename std::enable_if_t<std::is_same_v<Type, detail::TimeSliceBackgroundTask>, void>
        setTimeSliceThreadToUse (juce::TimeSliceThread* thread)
    {
        detail::TimeSliceBackgroundTask::setTimeSliceThreadToUse (thread);
    }

protected:
    /**
     * Override this method to prepare the child class.
     *
     * Make sure to set `blockSizeToRequest` with the block
     * size needed for the consuming process.
     *
     * If your process needs a custom refresh time, make sure to fill
     * in that field as well.
     */
    virtual void prepareTask (double /*sampleRate*/, int /*samplesPerBlock*/, int& blockSizeToRequest, int& customRefreshTimeMs) = 0;

    /**
     * If you task needs to reset any state, override
     * this method and reset the state here!
     */
    virtual void resetTask() {}

    /** Child classes must override this method to actually do the background task */
    virtual void runTask (const juce::AudioBuffer<float>& /*data*/) = 0;

private:
    int runTaskOnBackgroundThread() override;

    std::vector<DoubleBuffer<float>> data;
    std::atomic<int> writePosition { 0 };

    std::atomic_bool shouldBeRunning { false };
    std::atomic_bool isPrepared { false };

    int requestedDataSize = 0;
    int waitMilliseconds = 0;

    juce::AudioBuffer<float> latestData;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioUIBackgroundTask)
};

/** AudioUIBackgroundTask that will run on its own dedicated thread */
using SingleThreadAudioUIBackgroundTask = AudioUIBackgroundTask<detail::SingleThreadBackgroundTask>;

/** AudioUIBackgroundTask that will run in a time slice */
using TimeSliceAudioUIBackgroundTask = AudioUIBackgroundTask<detail::TimeSliceBackgroundTask>;
} // namespace chowdsp
