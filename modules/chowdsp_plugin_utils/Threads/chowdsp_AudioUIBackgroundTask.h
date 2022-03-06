#pragma once

// @TODO: this class needs chowdsp::DoubleBuffer, but we don't want
// chowdsp_plugin_utils to depend on chowdsp_dsp, so we just include
// the necessary header here. Eventually ,we should figure out a better
// way to do this...
#include "../../chowdsp_dsp/DataStructures/chowdsp_DoubleBuffer.h"

namespace chowdsp
{
/**
 * Let's say you need a class that can accept data from the audio thread,
 * do some computation with that data on a background thread, and then report
 * the computation result to the UI thread. This would be that class!
 *
 * The common scenario here is when you need a meter, or other audio visualization.
 */
class AudioUIBackgroundTask : public juce::Thread
{
public:
    /** Constructor with a name for the background thread */
    explicit AudioUIBackgroundTask (const juce::String& name);

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

    /** Child classes must override this method to actually do the background task */
    virtual void runTask (const juce::AudioBuffer<float>& /*data*/) = 0;

private:
    void run() override;

    std::vector<chowdsp::DoubleBuffer<float>> data;
    std::atomic<int> writePosition { 0 };

    std::atomic_bool shouldBeRunning { false };
    std::atomic_bool isPrepared { false };

    int requestedDataSize = 0;
    int waitMilliseconds = 0;

    juce::AudioBuffer<float> latestData;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioUIBackgroundTask)
};
} // namespace chowdsp
