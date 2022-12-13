#include <CatchUtils.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

namespace
{
constexpr double fs = 48000.0;
constexpr int nSamples = 512;
constexpr int pulseSpace = 100;
constexpr float delaySamp = 5.0f;
} // namespace

template <typename BypassType>
void processFunc (const chowdsp::BufferView<float>& buffer, BypassType& bypass, std::atomic<float>* onOffParam, const std::function<float (float)>& sampleFunc)
{
    auto onOff = bypass.toBool (onOffParam);
    if (! bypass.processBlockIn (buffer, onOff))
        return;

    auto* x = buffer.getWritePointer (0);
    for (int n = 0; n < nSamples; ++n)
        x[n] = sampleFunc (x[n]);

    bypass.processBlockOut (buffer, onOff);
}

static void checkForClicks (const float* buffer, const int numSamples, float thresh, const std::string& message)
{
    float prevSample = 0.0f;
    float maxDiff = 0.0f;
    for (int n = 0; n < numSamples; ++n)
    {
        auto diff = std::abs (buffer[n] - prevSample);
        maxDiff = juce::jmax (maxDiff, diff);
        prevSample = buffer[n];
    }

    REQUIRE_MESSAGE (maxDiff < thresh, message);
}

static void createPulseTrain (float* buffer, const int numSamples, int spacingSamples)
{
    for (int n = 0; n < numSamples; n += spacingSamples)
        buffer[n] = 1.0f;
}

static void checkPulseSpacing (const float* buffer, const int numSamples, int spacingSamples)
{
    int lastPulseIdx = (int) delaySamp;
    int numBadPulses = 0;
    int numGoodPulses = 0;
    for (int n = lastPulseIdx + 1; n < numSamples; ++n)
    {
        if (buffer[n] > 0.9f)
        {
            auto space = n - lastPulseIdx;
            if (space != spacingSamples)
            {
                INFO ("Incorrect spacing found! Start: " << lastPulseIdx << ", Length: " << space)
                numBadPulses++;
            }
            else
            {
                // std::cout << "Correct spacing found! Start: " << lastPulseIdx << ", Length: " << space << std::endl;
                numGoodPulses++;
            }
            lastPulseIdx = n;
        }
    }

    REQUIRE_MESSAGE (numBadPulses == 0, "Incorrect pulse spacing detected!");
    REQUIRE_MESSAGE (numGoodPulses == numSamples / spacingSamples, "Incorrect number of correct pulses!");
}

TEST_CASE ("Bypass Test")
{
    static constexpr int bufferTestNIters = 5;
    SECTION ("Audio Buffer Test")
    {
        chowdsp::BypassProcessor<float> bypass;
        std::atomic<float> onOffParam { 0.0f };
        bypass.prepare ({ fs, (juce::uint32) nSamples, 1 }, bypass.toBool (&onOffParam));

        chowdsp::Buffer<float> buffer (1, bufferTestNIters * nSamples);
        buffer.clear();
        for (int i = 0; i < bufferTestNIters; ++i)
        {
            chowdsp::BufferView<float> subBuffer { buffer, i * nSamples, nSamples };
            processFunc (subBuffer, bypass, &onOffParam, [] (float x)
                         { return x + 1.0f; });
            onOffParam.store (1.0f - onOffParam.load());
        }

        checkForClicks (buffer.getReadPointer (0), bufferTestNIters * nSamples, 0.005f, "Audio Buffer has clicks!");
    }

    static constexpr int delayTestNIters = 8;
    SECTION ("Audio Buffer Delay Test")
    {
        chowdsp::BypassProcessor<float, chowdsp::DelayLineInterpolationTypes::Linear> bypass;
        std::atomic<float> onOffParam { 0.0f };
        bypass.prepare ({ fs, (juce::uint32) nSamples, 1 }, bypass.toBool (&onOffParam));
        bypass.setLatencySamples (delaySamp);

        chowdsp::DelayLine<float> delay { 2048 };
        delay.prepare ({ fs, (juce::uint32) nSamples, 1 });
        delay.setDelay (delaySamp);

        chowdsp::Buffer<float> buffer (1, delayTestNIters * nSamples);
        createPulseTrain (buffer.getWritePointer (0), delayTestNIters * nSamples, pulseSpace);
        for (int i = 0; i < delayTestNIters; ++i)
        {
            chowdsp::BufferView<float> subBuffer { buffer, i * nSamples, nSamples };
            processFunc (subBuffer, bypass, &onOffParam, [&] (float x)
                         {
                delay.pushSample (0, x);
                return delay.popSample (0); });

            if (i % 2 != 0)
                onOffParam.store (1.0f - onOffParam.load());
        }

        checkPulseSpacing (buffer.getReadPointer (0), delayTestNIters * nSamples, pulseSpace);
    }
}
