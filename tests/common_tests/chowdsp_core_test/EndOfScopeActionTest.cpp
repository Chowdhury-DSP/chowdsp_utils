#include <CatchUtils.h>
#include <chowdsp_core/chowdsp_core.h>

TEST_CASE ("End Of Scope Action Test", "[common][functional]")
{
    SECTION ("Simple usage")
    {
        int x = 4;
        {
            const auto _ = chowdsp::runAtEndOfScope ([&x]
                                                     { x = 0; });
            REQUIRE (x == 4);
        }
        REQUIRE (x == 0);
    }

    SECTION ("Moved-from object doesn't invoke callback")
    {
        int lambdaCalled { 0 };
        {
            chowdsp::EndOfScopeAction endOfScopeAction { [&lambdaCalled]
                                                         { ++lambdaCalled; } };
            auto movedTo = std::move (endOfScopeAction);
            REQUIRE (lambdaCalled == 0);
        }
        REQUIRE (lambdaCalled == 1);
    }

    SECTION ("Moving into an existing object")
    {
        int action1Called { 0 };
        int action2Called { 0 };
        {
            [[maybe_unused]] chowdsp::EndOfScopeAction<std::function<void()>> action1 { [&action1Called]
                                                                                        { ++action1Called; } };

            [[maybe_unused]] chowdsp::EndOfScopeAction<std::function<void()>> action2 { [&action2Called]
                                                                                        { ++action2Called; } };

            action2 = std::move (action1);
            action1 = std::move (action2);

            REQUIRE (action1Called == 0);
            REQUIRE (action2Called == 1);
        }
        REQUIRE (action1Called == 1);
        REQUIRE (action2Called == 1);
    }
}
