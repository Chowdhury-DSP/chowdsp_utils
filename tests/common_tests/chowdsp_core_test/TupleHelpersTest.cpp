#include <CatchUtils.h>
#include <chowdsp_core/chowdsp_core.h>

TEST_CASE ("Tuple Helpers Test")
{
    SECTION ("forEachInTuple Test")
    {
        std::tuple<float, float, int, int> ints;

        chowdsp::TupleHelpers::forEachInTuple ([] (auto& x, size_t i)
                                               { x = (std::remove_reference_t<decltype (x)>) i; },
                                               ints);

        chowdsp::TupleHelpers::forEachInTuple ([] (auto& x, size_t i)
                                               { REQUIRE (x == (std::remove_reference_t<decltype (x)>) i); },
                                               ints);
    }
}
