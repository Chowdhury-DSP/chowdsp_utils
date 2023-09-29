#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>
#include <numeric>

TEMPLATE_TEST_CASE ("Vector Helpers Test", "[common][data-structures]", std::vector<int>, (chowdsp::SmallVector<int, 4>) )
{
    using Vector = TestType;

    SECTION ("Insert Sorted (Default)")
    {
        Vector vec { 1, 2, 5, 25 };
        chowdsp::insert_sorted (vec, 12);
        chowdsp::VectorHelpers::insert_sorted (vec, 3);

        for (const auto [exp, actual] : chowdsp::zip (vec, Vector { 1, 2, 3, 5, 12, 25 }))
            REQUIRE (exp == actual);
    }

    SECTION ("Insert Sorted (Custom)")
    {
        Vector vec { { 25 }, { 5 }, { 2 }, { 1 } };

        const auto pred = [] (const auto& x, const auto& y)
        { return x > y; };
        chowdsp::insert_sorted (vec, { 12 }, pred);
        chowdsp::VectorHelpers::insert_sorted (vec, { 3 }, pred);

        for (const auto [exp, actual] : chowdsp::zip (vec, Vector { 25, 12, 5, 3, 2, 1 }))
            REQUIRE (exp == actual);
    }

    SECTION ("Vector Erase Test")
    {
        const auto check_vec = [] (const Vector& vec, std::initializer_list<int> expected)
        {
            REQUIRE (vec.size() == expected.size());
        };

        Vector cnt (10);
        std::iota (cnt.begin(), cnt.end(), 0);
        check_vec (cnt, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 });

        chowdsp::erase (cnt, 3);
        check_vec (cnt, { 0, 1, 2, 4, 5, 6, 7, 8, 9 });

        auto erased = chowdsp::VectorHelpers::erase_if (cnt, [] (int x)
                                                        { return x % 2 == 0; });
        check_vec (cnt, { 1, 5, 7, 9 });
        REQUIRE (erased == 5);
    }
}
