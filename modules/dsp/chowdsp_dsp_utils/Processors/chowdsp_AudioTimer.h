#pragma once

namespace chowdsp
{
/** A timer to be used within audio callbacks */
class AudioTimer
{
public:
    /** Default constructor */
    AudioTimer() = default;

    /** Prepares the timer to run at a new sample rate. */
    void prepare (double sampleRate);

    /** Resets the timer back to zero. */
    void reset();

    /** Advances the timer by a given number of samples. */
    void advance (int numSamples);

    /** Return the current time in samples. */
    [[nodiscard]] juce::int64 getTimeSamples() const noexcept;

    /** Return the current time in milliseconds. */
    [[nodiscard]] double getTimeMilliseconds() const noexcept;

    /** Return the current time in seconds. */
    [[nodiscard]] double getTimeSeconds() const noexcept;

private:
    double sampleTimeSeconds = 0.0;
    double sampleTimeMilliseconds = 0.0;
    juce::int64 counter = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioTimer)
};
} // namespace chowdsp
