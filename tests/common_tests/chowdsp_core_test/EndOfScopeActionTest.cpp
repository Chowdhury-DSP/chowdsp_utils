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
}
