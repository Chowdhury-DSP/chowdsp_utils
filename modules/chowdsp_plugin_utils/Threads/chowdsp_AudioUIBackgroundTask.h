#pragma once

namespace chowdsp
{
class AudioUIBackgroundTask : public juce::Thread
{
public:
    explicit AudioUIBackgroundTask (const juce::String& name);

    void prepare (double sampleRate, int samplesPerBlock);

    virtual void prepareTask (double /*sampleRate*/, int /*samplesPerBlock*/, int& blockSizeToRequest) { blockSizeToRequest = 100; }

    virtual void pushSamples (const float* samples, int numSamples);

    void setShouldBeRunning (bool shouldRun);

    virtual void runTask (const float* /*data*/) = 0;

private:
    void run() override;

    chowdsp::DoubleBuffer<float> data;
    std::atomic<int> writePosition { 0 };

    std::atomic_bool shouldBeRunning { false };
    std::atomic_bool isPrepared { false };

    int requestedDataSize = 0;
    int waitMilliseconds = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioUIBackgroundTask)
};
} // namespace chowdsp
