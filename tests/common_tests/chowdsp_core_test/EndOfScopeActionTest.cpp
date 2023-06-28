#include <CatchUtils.h>
#include <chowdsp_core/chowdsp_core.h>

TEST_CASE ("End Of Scope Action Test", "[common][functional]")
{
    int x = 4;
    {
        const auto _ = chowdsp::runAtEndOfScope ([&x]
                                                 { x = 0; });
        REQUIRE (x == 4);
    }
    REQUIRE (x == 0);
}
