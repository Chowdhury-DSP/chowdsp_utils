#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

TEST_CASE ("Chained Arena Allocator Test", "[common][data-structures]")
{
    chowdsp::ChainedArenaAllocator allocator { 100 };

    SECTION ("Basic Usage")
    {
        // allocate doubles
        {
            auto* some_doubles = allocator.allocate<double> (10);
            REQUIRE ((void*) some_doubles == allocator.data<void> (0));
            REQUIRE (allocator.get_arena_count() == 1);
            REQUIRE (allocator.get_arenas().front().get_bytes_used() == 80);

            auto* some_more_doubles = allocator.allocate<double> (10);
            REQUIRE (some_more_doubles != nullptr);
            REQUIRE (allocator.get_arena_count() == 2);
            REQUIRE (allocator.get_arenas().front().get_bytes_used() == 80);
            REQUIRE ((allocator.get_arenas().begin()++)->get_bytes_used() == 80);
        }

        // overfull allocation
        REQUIRE (allocator.allocate<double> (200) == nullptr);

        // clear allocator
        allocator.clear();
        REQUIRE (allocator.get_arena_count() == 2);
        REQUIRE (allocator.get_current_arena().get_bytes_used() == 0);

        // re-allocate doubles
        {
            auto* some_doubles = allocator.allocate<double> (10);
            REQUIRE ((void*) some_doubles == allocator.data<void> (0));
            REQUIRE (allocator.get_arenas().front().get_bytes_used() == 80);

            auto* some_more_doubles = allocator.allocate<double> (10);
            REQUIRE (some_more_doubles != nullptr);
            REQUIRE (allocator.get_arena_count() == 2);
            REQUIRE (allocator.get_arenas().front().get_bytes_used() == 80);
            REQUIRE ((allocator.get_arenas().begin()++)->get_bytes_used() == 80);
        }
    }

    SECTION ("Usage with Frame")
    {
        {
            const auto frame = allocator.create_frame();

            auto* some_doubles = allocator.allocate<double> (10);
            REQUIRE ((void*) some_doubles == allocator.data<void> (0));
            REQUIRE (allocator.get_arena_count() == 1);
            REQUIRE (allocator.get_arenas().front().get_bytes_used() == 80);

            auto* some_more_doubles = allocator.allocate<double> (10);
            REQUIRE (some_more_doubles != nullptr);
            REQUIRE (allocator.get_arena_count() == 2);
            REQUIRE (allocator.get_arenas().front().get_bytes_used() == 80);
            REQUIRE ((allocator.get_arenas().begin()++)->get_bytes_used() == 80);
        }

        REQUIRE (allocator.get_arena_count() == 2);
        REQUIRE (allocator.get_arenas().front().get_bytes_used() == 0);
    }
}
