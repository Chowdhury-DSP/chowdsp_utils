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
            REQUIRE ((void*) some_doubles == allocator.template data<void> (0));
            REQUIRE (allocator.get_arena_count() == 1);
            REQUIRE (allocator.get_arenas().front().get_bytes_used() == 80);

            auto* some_more_doubles = allocator.allocate<double> (10);
            REQUIRE (some_more_doubles != nullptr);
            REQUIRE (allocator.get_arena_count() == 2);
            REQUIRE (allocator.get_arenas().front().get_bytes_used() == 80);
            REQUIRE ((allocator.get_arenas().begin()++)->get_bytes_used() == 80);
            REQUIRE (allocator.get_total_bytes_used() == 160);
        }

        // overfull allocation
        REQUIRE (allocator.template allocate<double> (200) == nullptr);

        // clear allocator
        allocator.clear();
        REQUIRE (allocator.get_arena_count() == 2);
        REQUIRE (allocator.get_current_arena().get_bytes_used() == 0);
        REQUIRE (allocator.get_total_bytes_used() == 0);

        // re-allocate doubles
        {
            auto* some_doubles = allocator.allocate<double> (10);
            REQUIRE ((void*) some_doubles == allocator.template data<void> (0));
            REQUIRE (allocator.get_arenas().front().get_bytes_used() == 80);
            REQUIRE (allocator.get_total_bytes_used() == 80);

            auto* some_more_doubles = allocator.allocate<double> (10);
            REQUIRE (some_more_doubles != nullptr);
            REQUIRE (allocator.get_arena_count() == 2);
            REQUIRE (allocator.get_arenas().front().get_bytes_used() == 80);
            REQUIRE ((allocator.get_arenas().begin()++)->get_bytes_used() == 80);
            REQUIRE (allocator.get_total_bytes_used() == 160);
        }
    }

    SECTION ("Usage with Frame")
    {
        {
            const auto frame = allocator.create_frame();

            auto* some_doubles = allocator.allocate<double> (10);
            REQUIRE ((void*) some_doubles == allocator.template data<void> (0));
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

    SECTION ("Merge")
    {
        chowdsp::ChainedArenaAllocator alloc1 { 128 };
        chowdsp::ChainedArenaAllocator alloc2 { 128 };
        chowdsp::ChainedArenaAllocator alloc3 {};

        alloc1.allocate<float> (24);
        alloc1.allocate<float> (24);
        alloc2.allocate<float> (24);
        alloc2.allocate<float> (24);
        alloc2.allocate<float> (24);

        alloc1.clear();
        alloc2.clear();

        chowdsp::ChainedArenaAllocator merge_alloc {};
        const auto arena_count = alloc1.get_arena_count() + alloc2.get_arena_count() + alloc3.get_arena_count();
        const auto* old_arena = &alloc1.get_current_arena();
        merge_alloc.merge (alloc1);
        alloc2.allocate<float> (24);
        alloc2.allocate<float> (24);
        merge_alloc.merge (alloc2);
        merge_alloc.merge (alloc3);
        REQUIRE (merge_alloc.get_arena_count() == arena_count);
        REQUIRE (&merge_alloc.get_current_arena() == old_arena);
    }
}
