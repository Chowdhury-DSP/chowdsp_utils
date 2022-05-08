#include <TimedUnitTest.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

namespace
{
constexpr double fs = 48000.0;
constexpr int nSamples = 512;
constexpr int pulseSpace = 100;
constexpr float delaySamp = 5.0f;
} // namespace

class BypassTest : public TimedUnitTest
{
public:
    BypassTest() : TimedUnitTest ("Bypass Test") {}

    [[maybe_unused]] static float* getBufferPtr (juce::AudioBuffer<float>& buffer)
    {
        return buffer.getWritePointer (0);
    }

    static float* getBufferPtr (juce::dsp::AudioBlock<float>& block)
    {
        return block.getChannelPointer (0);
    }

    template <typename AudioContainerType, typename BypassType>
    void processFunc (AudioContainerType& bufferOrBlock, BypassType& bypass, std::atomic<float>* onOffParam, const std::function<float (float)>& sampleFunc)
    {
        auto onOff = bypass.toBool (onOffParam);
        if (! bypass.processBlockIn (bufferOrBlock, onOff))
            return;

        auto* x = getBufferPtr (bufferOrBlock);
        for (int n = 0; n < nSamples; ++n)
            x[n] = sampleFunc (x[n]);

        bypass.processBlockOut (bufferOrBlock, onOff);
    }

    void checkForClicks (const float* buffer, const int numSamples, float thresh, const juce::String& message)
    {
        float prevSample = 0.0f;
        float maxDiff = 0.0f;
        for (int n = 0; n < numSamples; ++n)
        {
            auto diff = std::abs (buffer[n] - prevSample);
            maxDiff = juce::jmax (maxDiff, diff);
            prevSample = buffer[n];
        }

        expectLessThan (maxDiff, thresh, message);
    }

    static void createPulseTrain (float* buffer, const int numSamples, int spacingSamples)
    {
        for (int n = 0; n < numSamples; n += spacingSamples)
            buffer[n] = 1.0f;
    }

    void checkPulseSpacing (const float* buffer, const int numSamples, int spacingSamples)
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
                    std::cout << "Incorrect spacing found! Start: " << lastPulseIdx << ", Length: " << space << std::endl;
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

        expect (numBadPulses == 0, "Incorrect pulse spacing detected!");
        expectEquals (numGoodPulses, numSamples / spacingSamples, "Incorrect number of correct pulses!");
    }

    void audioBufferTest (int nIter)
    {
        chowdsp::BypassProcessor<float> bypass;
        std::atomic<float> onOffParam { 0.0f };
        bypass.prepare ({ fs, (juce::uint32) nSamples, 1 }, bypass.toBool (&onOffParam));

        juce::AudioBuffer<float> buffer (1, nIter * nSamples);
        buffer.clear();
        for (int i = 0; i < nIter; ++i)
        {
            juce::AudioBuffer<float> subBuffer (buffer.getArrayOfWritePointers(), 1, i * nSamples, nSamples);
            processFunc (subBuffer, bypass, &onOffParam, [] (float x) { return x + 1.0f; });
            onOffParam.store (1.0f - onOffParam.load());
        }

        checkForClicks (buffer.getReadPointer (0), nIter * nSamples, 0.005f, "Audio Buffer has clicks!");
    }

    void audioBlockTest (int nIter)
    {
        chowdsp::BypassProcessor<float> bypass;
        std::atomic<float> onOffParam { 0.0f };
        bypass.prepare ({ fs, (juce::uint32) nSamples, 1 }, bypass.toBool (&onOffParam));

        juce::AudioBuffer<float> buffer (1, nIter * nSamples);
        buffer.clear();
        juce::dsp::AudioBlock<float> block (buffer);
        for (int i = 0; i < nIter; ++i)
        {
            auto subBlock = block.getSubBlock ((size_t) i * (size_t) nSamples, (size_t) nSamples);
            processFunc (subBlock, bypass, &onOffParam, [] (float x) { return x + 1.0f; });
            onOffParam.store (1.0f - onOffParam.load());
        }

        checkForClicks (buffer.getReadPointer (0), nIter * nSamples, 0.005f, "Audio Block has clicks!");
    }

    void bufferDelayTest (int nIter)
    {
        chowdsp::BypassProcessor<float, chowdsp::DelayLineInterpolationTypes::Linear> bypass;
        std::atomic<float> onOffParam { 0.0f };
        bypass.prepare ({ fs, (juce::uint32) nSamples, 1 }, bypass.toBool (&onOffParam));
        bypass.setLatencySamples (delaySamp);

        chowdsp::DelayLine<float> delay { 2048 };
        delay.prepare ({ fs, (juce::uint32) nSamples, 1 });
        delay.setDelay (delaySamp);

        juce::AudioBuffer<float> buffer (1, nIter * nSamples);
        buffer.clear();
        createPulseTrain (buffer.getWritePointer (0), nIter * nSamples, pulseSpace);
        for (int i = 0; i < nIter; ++i)
        {
            juce::AudioBuffer<float> subBuffer (buffer.getArrayOfWritePointers(), 1, i * nSamples, nSamples);
            processFunc (subBuffer, bypass, &onOffParam, [&] (float x) {
                delay.pushSample (0, x);
                return delay.popSample (0); });

            if (i % 2 != 0)
                onOffParam.store (1.0f - onOffParam.load());
        }

        checkPulseSpacing (buffer.getReadPointer (0), nIter * nSamples, pulseSpace);
    }

    void blockDelayTest (int nIter)
    {
        chowdsp::BypassProcessor<float> bypass;
        std::atomic<float> onOffParam { 0.0f };
        bypass.prepare ({ fs, (juce::uint32) nSamples, 1 }, bypass.toBool (&onOffParam));
        bypass.setLatencySamples ((int) delaySamp);

        chowdsp::DelayLine<float> delay { 2048 };
        delay.prepare ({ fs, (juce::uint32) nSamples, 1 });
        delay.setDelay (delaySamp);

        juce::AudioBuffer<float> buffer (1, nIter * nSamples);
        buffer.clear();
        createPulseTrain (buffer.getWritePointer (0), nIter * nSamples, pulseSpace);
        juce::dsp::AudioBlock<float> block (buffer);
        for (int i = 0; i < nIter; ++i)
        {
            auto subBlock = block.getSubBlock ((size_t) i * (size_t) nSamples, (size_t) nSamples);
            processFunc (subBlock, bypass, &onOffParam, [&] (float x) {
                delay.pushSample (0, x);
                return delay.popSample (0); });

            if (i % 2 != 0)
                onOffParam.store (1.0f - onOffParam.load());
        }

        checkPulseSpacing (buffer.getReadPointer (0), nIter * nSamples, pulseSpace);
    }

    void runTestTimed() override
    {
        beginTest ("Audio Buffer Test");
        audioBufferTest (5);

        beginTest ("Audio Block Test");
        audioBlockTest (5);

        beginTest ("Audio Buffer Delay Test");
        bufferDelayTest (8);

        beginTest ("Audio Block Delay Test");
        blockDelayTest (8);
    }
};

static BypassTest bypassTest;
