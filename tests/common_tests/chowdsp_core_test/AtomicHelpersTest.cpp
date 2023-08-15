#include <CatchUtils.h>
#include <chowdsp_core/chowdsp_core.h>
#include <chrono>
#include <thread>

TEST_CASE ("Atomic Helpers Test", "[common][data-structures]")
{
    SECTION ("Compare/Exchange Test")
    {
        using chowdsp::AtomicHelpers::compareExchange;
        {
            std::atomic<float> f { 0.0f };
            REQUIRE_MESSAGE (compareExchange (f, 0.0f, 1.0f), "Compare/Exchange should return true!");
            REQUIRE_MESSAGE (juce::exactlyEqual (f.load(), 1.0f), "Compare/Exchange value update is incorrect!");

            REQUIRE_MESSAGE (! compareExchange (f, 0.0f, 1.0f), "Compare/Exchange should return false!");
            REQUIRE_MESSAGE (juce::exactlyEqual (f.load(), 1.0f), "Compare/Exchange value update is happening on false case!");
        }

        {
            int i = 0;
            chowdsp::AtomicRef iRef { i };

            REQUIRE_MESSAGE (compareExchange (iRef, 0, 1), "Compare/Exchange should return true!");
            REQUIRE_MESSAGE (iRef.load() == 1, "Compare/Exchange value update is incorrect!");

            REQUIRE_MESSAGE (! compareExchange (iRef, 0, 1), "Compare/Exchange should return false!");
            REQUIRE_MESSAGE (iRef.load() == 1, "Compare/Exchange value update is happening on false case!");
        }
    }

    SECTION ("Compare/Negate Test")
    {
        using chowdsp::AtomicHelpers::compareNegate;
        {
            std::atomic<bool> b { true };
            REQUIRE_MESSAGE (compareNegate (b), "Compare/Negate should return true!");
            REQUIRE_MESSAGE (! b.load(), "Compare/Negate value update is incorrect!");

            REQUIRE_MESSAGE (! compareNegate (b), "Compare/Negate should return false!");
            REQUIRE_MESSAGE (! b.load(), "Compare/Negate value update is happening on false case!");
        }

        {
            bool b = false;
            chowdsp::AtomicRef<bool> bRef { b };
            REQUIRE_MESSAGE (compareNegate (bRef, false), "Compare/Negate should return true!");
            REQUIRE_MESSAGE (bRef.load(), "Compare/Negate value update is incorrect!");

            REQUIRE_MESSAGE (! compareNegate (bRef, false), "Compare/Negate should return false!");
            REQUIRE_MESSAGE (bRef.load(), "Compare/Negate value update is happening on false case!");
        }
    }

    SECTION ("AtomicRef Test")
    {
        float x = 0.0f;

        std::thread thread1 (
            [&x]
            {
                chowdsp::AtomicRef xFreq { x };
                for (int i = 0; i < 100000; ++i)
                {
                    xFreq.store ((float) i);
                }
            });

        std::thread thread2 (
            [&x]
            {
                chowdsp::AtomicRef xFreq { std::as_const (x) };
                while (xFreq.load() < 90000.0f)
                {
                    std::this_thread::sleep_for (std::chrono::milliseconds (10));
                }
            });

        thread1.join();
        thread2.join();
    }

    SECTION ("Fetch Or")
    {
        bool x = false;
        std::atomic_bool x_atomic { false };

        x |= true;
        chowdsp::AtomicHelpers::fetch_or (x_atomic, true);
        REQUIRE (x == x_atomic.load());

        REQUIRE ((x | false) == chowdsp::AtomicHelpers::fetch_or (x_atomic, false));
    }

    SECTION ("Fetch And")
    {
        bool x = false;
        std::atomic_bool x_atomic { false };

        x &= true;
        chowdsp::AtomicHelpers::fetch_and (x_atomic, true);
        REQUIRE (x == x_atomic.load());

        REQUIRE ((x & false) == chowdsp::AtomicHelpers::fetch_and (x_atomic, false));
    }
}
