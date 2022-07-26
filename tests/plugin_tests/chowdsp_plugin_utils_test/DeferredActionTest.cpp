#include <TimedUnitTest.h>
#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>

class DeferredActionTest : public TimedUnitTest
{
public:
    DeferredActionTest() : TimedUnitTest ("Deferred Action Test", "Threads")
    {
    }

    void deferredCounterIncrementTest (bool fakeAudioThread = false)
    {
        struct Counter
        {
            explicit Counter (juce::UnitTest& test) : ut (test) {}

            juce::UnitTest& ut;
            int count = 0;

            void increment()
            {
                ut.expect (juce::MessageManager::existsAndIsCurrentThread(), "Deferred action was called from a thread other than the message thread!");
                count++;
            }
        };

        chowdsp::DeferredAction action;
        Counter counter { *this };
        std::atomic<int> refCounter;

        juce::Thread::launch (
            [&]
            {
                for (int i = 0; i < 100; ++i)
                {
                    action.call ([&counter]
                                 { counter.increment(); },
                                 fakeAudioThread);
                    refCounter++;
                    juce::Thread::sleep (5);
                }
            });

        for (int i = 0; i < 100; ++i)
        {
            action.call ([&counter]
                         { counter.increment(); });
            refCounter++;
            juce::MessageManager::getInstance()->runDispatchLoopUntil (5);
        }

        juce::MessageManager::getInstance()->runDispatchLoopUntil (100); // clear up any remaining async updates

        expectEquals (counter.count, refCounter.load(), "Final count is incorrect!");
    }

    void runTestTimed() override
    {
        beginTest ("Deferred Action Test");
        deferredCounterIncrementTest();

        beginTest ("Deferred Action from Audio Thread Test");
        deferredCounterIncrementTest (true);
    }
};

static DeferredActionTest deferredActionTest;
