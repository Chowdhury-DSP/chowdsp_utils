#include <TimedUnitTest.h>
#include <juce_events/juce_events.h>
#include <chowdsp_core/chowdsp_core.h>

class AtomicHelpersTest : public TimedUnitTest
{
public:
    AtomicHelpersTest() : TimedUnitTest ("Atomic Helpers Test") {}

    void compareExchangeTest()
    {
        using chowdsp::AtomicHelpers::compareExchange;
        {
            std::atomic<float> f { 0.0f };
            expect (compareExchange (f, 0.0f, 1.0f), "Compare/Exchange should return true!");
            expectEquals (f.load(), 1.0f, "Compare/Exchange value update is incorrect!");

            expect (! compareExchange (f, 0.0f, 1.0f), "Compare/Exchange should return false!");
            expectEquals (f.load(), 1.0f, "Compare/Exchange value update is happening on false case!");
        }

        {
            int i = 0;
            chowdsp::AtomicRef iRef { i };

            expect (compareExchange (iRef, 0, 1), "Compare/Exchange should return true!");
            expectEquals (iRef.load(), 1, "Compare/Exchange value update is incorrect!");

            expect (! compareExchange (iRef, 0, 1), "Compare/Exchange should return false!");
            expectEquals (iRef.load(), 1, "Compare/Exchange value update is happening on false case!");
        }
    }

    void compareNegateTest()
    {
        using chowdsp::AtomicHelpers::compareNegate;
        {
            std::atomic<bool> b { true };
            expect (compareNegate (b), "Compare/Negate should return true!");
            expect (! b.load(), "Compare/Negate value update is incorrect!");

            expect (! compareNegate (b), "Compare/Negate should return false!");
            expect (! b.load(), "Compare/Negate value update is happening on false case!");
        }

        {
            bool b = false;
            chowdsp::AtomicRef<bool> bRef { b };
            expect (compareNegate (bRef, false), "Compare/Negate should return true!");
            expect (bRef.load(), "Compare/Negate value update is incorrect!");

            expect (! compareNegate (bRef, false), "Compare/Negate should return false!");
            expect (bRef.load(), "Compare/Negate value update is happening on false case!");
        }
    }

    void atomicRefTest()
    {
        float x = 0.0f;

        std::atomic<bool> thread1Finished { false };
        juce::Thread::launch (
            [&] {
                chowdsp::AtomicRef xFreq { x };
                for (int i = 0; i < 100000; ++i)
                {
                    xFreq.store ((float) i);
                }

                // do stuff on background thread
                thread1Finished = true;
            });

        std::atomic<bool> thread2Finished { false };
        juce::Thread::launch (
            [&] {
                chowdsp::AtomicRef xFreq { std::as_const (x) };
                while (xFreq.load() < 90000.0f)
                {
                    juce::Thread::sleep (10);
                }

                // do stuff on background thread
                thread2Finished = true;
            });

        while (! (thread1Finished && thread2Finished))
            juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
    }

    void runTestTimed() override
    {
        beginTest ("Compare/Exchange Test");
        compareExchangeTest();

        beginTest ("Compare/Negate Test");
        compareNegateTest();

        beginTest ("AtomicRef Test");
        atomicRefTest();
    }
};

static AtomicHelpersTest atomicHelpersTest;
