#include <CatchUtils.h>
#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>

static void deferredCounterIncrementTest (bool fakeAudioThread = false)
{
    struct Counter
    {
        int count = 0;

        void increment()
        {
            REQUIRE_MESSAGE (juce::MessageManager::existsAndIsCurrentThread(), "Deferred action was called from a thread other than the message thread!");
            count++;
        }
    };

    chowdsp::DeferredAction action;
    Counter counter;
    std::atomic<int> refCounter { 0 };
    std::atomic_bool isFinished { false };

    juce::Thread::launch (
        [&]
        {
            for (int i = 0; i < 25; ++i)
            {
                action.call ([&counter]
                             { counter.increment(); },
                             fakeAudioThread);
                refCounter.fetch_add (1);
                juce::Thread::sleep (12);
            }
            isFinished.store (true);
        });

    for (int i = 0; i < 25; ++i)
    {
        action.call ([&counter]
                     { counter.increment(); });
        refCounter.fetch_add (1);
        juce::MessageManager::getInstance()->runDispatchLoopUntil (15);
    }

    while (! isFinished.load())
        juce::MessageManager::getInstance()->runDispatchLoopUntil (500); // clear up any remaining async updates

    REQUIRE_MESSAGE (counter.count == refCounter.load(), "Final count is incorrect!");
}

TEST_CASE ("Deferred Action Test", "[plugin][utilities]")
{
    SECTION ("Deferred Action Test")
    {
        deferredCounterIncrementTest();
    }

    SECTION ("Deferred Action From Audio Thread Test")
    {
        deferredCounterIncrementTest (true);
    }
}
