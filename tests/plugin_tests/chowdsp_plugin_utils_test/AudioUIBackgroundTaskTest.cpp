#include <CatchUtils.h>
#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>

namespace
{
constexpr std::array<float, 9> mags { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f };
constexpr int blockSize = 1 << 19;

template <typename Task>
struct SimpleTask : Task
{
    explicit SimpleTask() : Task ("Basic Task")
    {
    }

    void prepareTask (double, int, int& requestedBlockSize, int& /*waitMs*/) override
    {
        requestedBlockSize = blockSize;
    }

    void runTask (const juce::AudioBuffer<float>& buffer) override
    {
        if (*magsIndex == mags.size() - 1)
            return;

        mag = buffer.getMagnitude (0, buffer.getNumSamples());

        if (wasReset)
        {
            REQUIRE_MESSAGE (mag == Catch::Approx { 0.0f }.margin (0.1f), "Magnitude after reset is incorrect!");
            wasReset.store (false);
        }
        else
        {
            REQUIRE_MESSAGE (mag == Catch::Approx { mags[prevIndex] }.margin (0.1f), "Magnitude is incorrect!");
        }

        prevIndex.store (magsIndex->load());
        readyForInc = true;
    }

    std::atomic_bool readyForInc { false };
    std::atomic<size_t> prevIndex {};
    std::atomic<size_t>* magsIndex = nullptr;
    float mag = 0.0f;
    std::atomic_bool wasReset { false };
};
} // namespace

template <typename TaskType>
class AudioUIBackgroundTaskTest
{
public:
    AudioUIBackgroundTaskTest() = default;

    void audioThreadTest (juce::TimeSliceThread* timeSliceThread = nullptr)
    {
        std::atomic<size_t> magsIndex {};

        SimpleTask<TaskType> task;
        task.magsIndex = &magsIndex;
        REQUIRE_MESSAGE (! task.isTaskRunning(), "Task should not be running yet!");

        if constexpr (std::is_same<TaskType, chowdsp::TimeSliceAudioUIBackgroundTask>::value)
        {
            if (timeSliceThread != nullptr)
                task.setTimeSliceThreadToUse (timeSliceThread);
        }

        task.prepare (48.0e3, blockSize, 1);
        task.setShouldBeRunning (true);
        REQUIRE_MESSAGE (task.isTaskRunning(), "Task should now be running!");

        std::vector<float> data ((size_t) blockSize, 0.0f);
        while (magsIndex < mags.size() - 1)
        {
            if (task.readyForInc)
            {
                magsIndex = juce::jmax (magsIndex + 1, mags.size() - 1);
                task.readyForInc = false;
            }

            std::fill (data.begin(), data.end(), mags[magsIndex]);
            task.pushSamples (0, data.data(), blockSize);
        }

        task.setShouldBeRunning (false);
        REQUIRE_MESSAGE (! task.isTaskRunning(), "Task should no longer be running!");
    }

    void guiThreadTest()
    {
        std::atomic<size_t> magsIndex {};

        SimpleTask<TaskType> task;
        task.magsIndex = &magsIndex;

        task.prepare (24.0e3, 128, 1);
        task.setShouldBeRunning (true);
        task.prepare (48.0e3, blockSize, 2);

        std::atomic_bool uiThreadFinished { false };
        juce::Thread::launch ([&] { // fake UI thread
            while (magsIndex < mags.size() / 2)
            {
                REQUIRE_MESSAGE (task.mag == Catch::Approx { mags[task.prevIndex] }.margin (0.1f), "Magnitude read from UI thread is incorrect!");
                juce::Thread::sleep (1);
            }

            task.reset();
            task.wasReset.store (true);
            REQUIRE_MESSAGE (task.mag == Catch::Approx { 0.0f }.margin (0.1f), "Magnitude after reset is incorrect!");

            while (magsIndex < mags.size() - 1)
            {
                REQUIRE_MESSAGE (task.mag == Catch::Approx { mags[task.prevIndex] }.margin (0.1f), "Magnitude read from UI thread is incorrect!");
                juce::Thread::sleep (1);
            }
            uiThreadFinished = true;
        });

        juce::AudioBuffer<float> data (2, blockSize);
        while (magsIndex < mags.size() - 1)
        {
            if (task.readyForInc)
            {
                magsIndex = juce::jmax (magsIndex + 1, mags.size() - 1);
                task.readyForInc = false;
            }

            auto* dataPtrL = data.getWritePointer (0);
            auto* dataPtrR = data.getWritePointer (1);
            std::fill (dataPtrL, dataPtrL + blockSize, mags[magsIndex]);
            std::fill (dataPtrR, dataPtrR + blockSize, mags[magsIndex]);
            task.pushSamples (data);
        }

        task.setShouldBeRunning (false);

        while (! uiThreadFinished)
            juce::Thread::sleep (1);
    }

    void customTimeSliceThreadTest()
    {
        std::atomic<size_t> magsIndex {};
        juce::TimeSliceThread timeSliceThread { "Test Thread" };

        SimpleTask<TaskType> task;
        task.magsIndex = &magsIndex;
        REQUIRE_MESSAGE (! task.isTaskRunning(), "Task should not be running yet!");

        task.prepare (48.0e3, blockSize, 1);
        task.setShouldBeRunning (true);
        REQUIRE_MESSAGE (task.isTaskRunning(), "Task should now be running!");

        task.setTimeSliceThreadToUse (&timeSliceThread);

        audioThreadTest (&timeSliceThread);

        task.setShouldBeRunning (false);
    }
};

TEST_CASE ("Single Thread Audio/UI Background Task Test", "[plugin][utilities]")
{
    AudioUIBackgroundTaskTest<chowdsp::SingleThreadAudioUIBackgroundTask> singleThreadTest;

    SECTION ("Audio Thread Test")
    {
        singleThreadTest.audioThreadTest();
    }

    SECTION ("GUI Thread Test")
    {
        singleThreadTest.guiThreadTest();
    }
}

TEST_CASE ("Time Slice Audio/UI Background Task Test", "[plugin][utilities]")
{
    AudioUIBackgroundTaskTest<chowdsp::TimeSliceAudioUIBackgroundTask> timeSliceTest;

    SECTION ("Audio Thread Test")
    {
        timeSliceTest.audioThreadTest();
    }

    SECTION ("GUI Thread Test")
    {
        timeSliceTest.guiThreadTest();
    }

    SECTION ("Custom TimeSliceThread Test")
    {
        timeSliceTest.customTimeSliceThreadTest();
    }
}
