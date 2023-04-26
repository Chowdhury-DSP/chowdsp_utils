#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>
#include <numeric>

TEST_CASE ("Vector Helpers Test", "[common][data-structures]")
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

        const auto pred = [] (const auto& x, const auto& y)
        { return x.x < y.x; };
        chowdsp::VectorHelpers::insert_sorted (vec, { 12 }, pred);
        chowdsp::VectorHelpers::insert_sorted (vec, { 3 }, pred);

        for (const auto [exp, actual] : chowdsp::zip (vec, std::vector { 1, 2, 3, 5, 12, 25 }))
            REQUIRE (exp.x == actual);
    }

    SECTION ("Vector Erase Test")
    {
        const auto check_vec = [] (const std::vector<int>& vec, std::initializer_list<int> expected)
        {
            REQUIRE (vec.size() == expected.size());
        };

        std::vector<int> cnt (10);
        std::iota (cnt.begin(), cnt.end(), 0);
        check_vec (cnt, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 });

        chowdsp::VectorHelpers::erase (cnt, 3);
        check_vec (cnt, { 0, 1, 2, 4, 5, 6, 7, 8, 9 });

        auto erased = chowdsp::VectorHelpers::erase_if (cnt, [] (int x)
                                                        { return x % 2 == 0; });
        check_vec (cnt, { 1, 5, 7, 9 });
        REQUIRE (erased == 5);
    }
}
