#include "chowdsp_AudioTimer.h"

namespace chowdsp
{
void AudioTimer::prepare (double sampleRate)
{
    sampleTimeSeconds = 1.0 / sampleRate;
    sampleTimeMilliseconds = 1000.0 * sampleTimeSeconds;

    reset();
}

void AudioTimer::reset()
{
    counter = 0;
}

void AudioTimer::advance (int numSamples)
{
    counter += numSamples;
}

juce::int64 AudioTimer::getTimeSamples() const noexcept
{
    return counter;
}

double AudioTimer::getTimeMilliseconds() const noexcept
{
    return (double) counter * sampleTimeMilliseconds;
}

double AudioTimer::getTimeSeconds() const noexcept
{
    return (double) counter * sampleTimeSeconds;
}
} // namespace chowdsp
