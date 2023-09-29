#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

TEST_CASE ("Tuple Helpers Test", "[common][data-structures]")
{
    SECTION ("forEachInTuple Test")
    {
        std::tuple<float, float, int, int> ints;

        chowdsp::forEachInTuple ([] (auto& x, size_t i)
                                 { x = (std::remove_reference_t<decltype (x)>) i; },
                                 ints);

        chowdsp::TupleHelpers::forEachInTuple ([] (auto& x, size_t i)
                                               { REQUIRE (juce::exactlyEqual (x, (std::remove_reference_t<decltype (x)>) i)); },
                                               ints);
    }

    SECTION ("visit_at Test")
    {
        struct Test1
        {
            void set (int v) { x = v; }
            int x;
        };

        struct Test2
        {
            void set (int v) { x = (float) v; }
            float x;
        };

        std::tuple<Test1, Test2> tuple;

        chowdsp::visit_at (tuple, 0, [] (auto& x)
                           { x.set (4); });
        chowdsp::visit_at (tuple, 1, [] (auto& x)
                           { x.set (5); });

        REQUIRE (std::get<0> (tuple).x == 4);
        REQUIRE (juce::exactlyEqual (std::get<1> (tuple).x, 5.0f));
    }

    SECTION ("const visit_at Test")
    {
        struct Test1
        {
            int x = 10;
        };

        struct Test2
        {
            float x = 100.0f;
        };

        std::tuple<Test1, Test2> tuple;

        int sum = 0;
        for (size_t i = 0; i < std::tuple_size<decltype (tuple)>(); ++i)
            chowdsp::TupleHelpers::visit_at (tuple, i, [&sum] (auto& x)
                                             { sum += (int) x.x; });

        REQUIRE (sum == 110);
    }
}
