#include <CatchUtils.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

namespace
{
constexpr double fs = 48000.0;
constexpr int nSamples = 512;
constexpr int pulseSpace = 100;
constexpr float delaySamp = 5.0f;
} // namespace

template <typename BypassType, typename FuncType>
void processFunc (const chowdsp::BufferView<float>& buffer,
                  BypassType& bypass,
                  std::atomic<float>* onOffParam,
                  chowdsp::ArenaAllocatorView* arena,
                  FuncType&& blockFunc)
{
    std::optional<chowdsp::ArenaAllocatorView::Frame> arena_frame;
    if (arena != nullptr)
        arena_frame.emplace (*arena);

    auto onOff = bypass.toBool (onOffParam);
    if (! bypass.processBlockIn (buffer,
                                 onOff,
                                 arena == nullptr ? std::optional<chowdsp::ArenaAllocatorView> { std::nullopt } : *arena))
        return;

    blockFunc (buffer);
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
                INFO ("Incorrect spacing found! Start: " << lastPulseIdx << ", Length: " << space);
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

TEST_CASE ("Bypass Test", "[dsp][misc]")
{
    static constexpr int bufferTestNIters = 5;
    SECTION ("Audio Buffer Test")
    {
        auto arena = chowdsp::ArenaAllocator<std::array<std::byte, nSamples * sizeof (float) + 32>> {};
        auto arenaView = chowdsp::ArenaAllocatorView { arena };
        for (auto& arenaPtr : std::vector<chowdsp::ArenaAllocatorView*> { nullptr, &arenaView })
        {
            chowdsp::BypassProcessor<float> bypass {};
            std::atomic<float> onOffParam { 0.0f };
            bypass.prepare ({ fs,
                              (juce::uint32) nSamples,
                              1 },
                            bypass.toBool (&onOffParam),
                            arenaPtr == nullptr);

            chowdsp::Buffer<float> buffer (1, bufferTestNIters * nSamples);
            buffer.clear();
            for (int i = 0; i < bufferTestNIters; ++i)
            {
                chowdsp::BufferView<float> subBuffer { buffer, i * nSamples, nSamples };
                processFunc (subBuffer,
                             bypass,
                             &onOffParam,
                             arenaPtr,
                             [] (const chowdsp::BufferView<float>& block)
                             {
                                 for (const auto& [ch, data] : chowdsp::buffer_iters::channels (block))
                                     juce::FloatVectorOperations::add (data.data(), 1.0f, data.size());
                             });
                onOffParam.store (1.0f - onOffParam.load());
            }

            checkForClicks (buffer.getReadPointer (0), bufferTestNIters * nSamples, 0.005f, "Audio Buffer has clicks!");
        }
    }

    static constexpr int delayTestNIters = 8;
    SECTION ("Audio Buffer Delay Test")
    {
        auto arena = chowdsp::ArenaAllocator<std::array<std::byte, nSamples * sizeof (float) + 32>> {};
        auto arenaView = chowdsp::ArenaAllocatorView { arena };
        for (auto& arenaPtr : std::vector<chowdsp::ArenaAllocatorView*> { nullptr, &arenaView })
        {
            chowdsp::BypassProcessor<float, chowdsp::DelayLineInterpolationTypes::Linear> bypass { static_cast<int> (std::ceil (delaySamp)) };
            std::atomic<float> onOffParam { 0.0f };
            bypass.prepare ({ fs,
                              (juce::uint32) nSamples,
                              1 },
                            bypass.toBool (&onOffParam),
                            arenaPtr == nullptr);
            bypass.setLatencySamples (delaySamp);

            chowdsp::DelayLine<float> delay { 2048 };
            delay.prepare ({ fs, (juce::uint32) nSamples, 1 });
            delay.setDelay (delaySamp);

            chowdsp::Buffer<float> buffer (1, delayTestNIters * nSamples);
            createPulseTrain (buffer.getWritePointer (0), delayTestNIters * nSamples, pulseSpace);
            for (int i = 0; i < delayTestNIters; ++i)
            {
                chowdsp::BufferView<float> subBuffer { buffer, i * nSamples, nSamples };
                processFunc (subBuffer,
                             bypass,
                             &onOffParam,
                             arenaPtr,
                             [&] (const chowdsp::BufferView<float>& block)
                             {
                                 delay.processBlock (block);
                             });

                if (i % 2 != 0)
                    onOffParam.store (1.0f - onOffParam.load());
            }
            delay.free();

            checkPulseSpacing (buffer.getReadPointer (0), delayTestNIters * nSamples, pulseSpace);
        }
    }
}
