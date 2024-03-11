#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

TEMPLATE_TEST_CASE ("Chained Arena Allocator Test", "[common][data-structures]", (std::vector<std::byte>), (std::array<std::byte, 100>) )
{
    using MemoryResourceType = TestType;

    auto allocator = []
    {
        if constexpr (std::is_same_v<MemoryResourceType, std::vector<std::byte>>)
            return chowdsp::ChainedArenaAllocator<MemoryResourceType> { 100 };
        else
            return chowdsp::ChainedArenaAllocator<MemoryResourceType> {};
    }();

    SECTION ("Basic Usage")
    {
        // allocate doubles
        {
            auto* some_doubles = allocator.template allocate<double> (10);
            REQUIRE ((void*) some_doubles == allocator.template data<void> (0));
            REQUIRE (allocator.get_arena_count() == 1);
            REQUIRE (allocator.get_arenas().front().get_bytes_used() == 80);

            auto* some_more_doubles = allocator.template allocate<double> (10);
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
            auto* some_doubles = allocator.template allocate<double> (10);
            REQUIRE ((void*) some_doubles == allocator.template data<void> (0));
            REQUIRE (allocator.get_arenas().front().get_bytes_used() == 80);
            REQUIRE (allocator.get_total_bytes_used() == 80);

            auto* some_more_doubles = allocator.template allocate<double> (10);
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

            auto* some_doubles = allocator.template allocate<double> (10);
            REQUIRE ((void*) some_doubles == allocator.template data<void> (0));
            REQUIRE (allocator.get_arena_count() == 1);
            REQUIRE (allocator.get_arenas().front().get_bytes_used() == 80);

            auto* some_more_doubles = allocator.template allocate<double> (10);
            REQUIRE (some_more_doubles != nullptr);
            REQUIRE (allocator.get_arena_count() == 2);
            REQUIRE (allocator.get_arenas().front().get_bytes_used() == 80);
            REQUIRE ((allocator.get_arenas().begin()++)->get_bytes_used() == 80);
        }

        REQUIRE (allocator.get_arena_count() == 2);
        REQUIRE (allocator.get_arenas().front().get_bytes_used() == 0);
    }
}
