#include <CatchUtils.h>
#include <chowdsp_core/chowdsp_core.h>

TEST_CASE ("Vector Helpers Test")
{
    SECTION ("Insert Sorted (Default)")
    {
        std::vector<int> vec { 1, 2, 5, 25 };
        chowdsp::VectorHelpers::insert_sorted (vec, 12);
        chowdsp::VectorHelpers::insert_sorted (vec, 3);

        for (const auto [exp, actual] : chowdsp::zip (vec, std::vector { 1, 2, 3, 5, 12, 25 }))
            REQUIRE (exp == actual);
    }

    SECTION ("Insert Sorted (Custom)")
    {
        struct JustAnInt
        {
            int x;
        };

        std::vector<JustAnInt> vec { { 1 }, { 2 }, { 5 }, { 25 } };

        const auto pred = [](const auto& x, const auto& y) { return x.x < y.x; };
        chowdsp::VectorHelpers::insert_sorted (vec, { 12 }, pred);
        chowdsp::VectorHelpers::insert_sorted (vec, { 3 }, pred);

        for (const auto [exp, actual] : chowdsp::zip (vec, std::vector { 1, 2, 3, 5, 12, 25 }))
            REQUIRE (exp.x == actual);
    }
}
