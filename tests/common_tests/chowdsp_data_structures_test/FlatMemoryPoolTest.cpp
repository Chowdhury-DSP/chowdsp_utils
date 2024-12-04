#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

TEST_CASE ("Flat Memory Pool Test", "[common][data-structures]")
{
    chowdsp::FlatMemoryPool allocator {};
    allocator.init();

    SECTION ("Basic Usage")
    {
        // allocate doubles
        double* some_doubles;
        {
            some_doubles = chowdsp::arena::allocate<double> (allocator, 10);
            REQUIRE (some_doubles != nullptr);
            REQUIRE (allocator.get_bytes_used() == 80);

            auto* some_more_doubles = chowdsp::arena::allocate<double> (allocator, 10);
            REQUIRE (some_more_doubles != nullptr);
            REQUIRE (some_more_doubles == some_doubles + 10);
            REQUIRE (allocator.get_bytes_used() == 160);
        }

        // clear allocator
        allocator.clear();

        // re-allocate doubles
        {
            auto* some_doubles2 = chowdsp::arena::allocate<double> (allocator, 10);
            REQUIRE (some_doubles2 != nullptr);
            REQUIRE (some_doubles2 == some_doubles);
            REQUIRE (allocator.get_bytes_used() == 80);

            auto* some_more_doubles = chowdsp::arena::allocate<double> (allocator, 10);
            REQUIRE (some_more_doubles != nullptr);
            REQUIRE (some_more_doubles == some_doubles + 10);
            REQUIRE (allocator.get_bytes_used() == 160);
        }
    }

    SECTION ("Overfull Allocation")
    {
        REQUIRE (chowdsp::arena::allocate<double> (allocator, 20'000) != nullptr);
        REQUIRE (chowdsp::arena::allocate<double> (allocator, 200) != nullptr);
        REQUIRE (chowdsp::arena::allocate<double> (allocator, 2'000'000) != nullptr);
        REQUIRE (allocator.get_bytes_used() == 8 * 2'020'200);
        allocator.clear();
        REQUIRE (allocator.get_bytes_used() == 0);
    }

    SECTION ("Usage with Frame")
    {
        {
            const auto frame = allocator.create_frame();

            auto* some_doubles = chowdsp::arena::allocate<double> (allocator, 10);
            REQUIRE (some_doubles != nullptr);
            REQUIRE (allocator.get_bytes_used() == 80);

            auto* some_more_doubles = chowdsp::arena::allocate<double> (allocator, 10);
            REQUIRE (some_more_doubles != nullptr);
            REQUIRE (allocator.get_bytes_used() == 160);
        }

        REQUIRE (allocator.get_bytes_used() == 0);

        allocator.deinit();
        REQUIRE (allocator.memory_base == nullptr);
    }
}
