#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

TEST_CASE ("Pool Allocator Test", "[common][data-structures]")
{
    SECTION ("Default")
    {
        chowdsp::PoolAllocator<32, 8> allocator {};
        REQUIRE (allocator.backing_buffer.empty());
    }

    SECTION ("With Size")
    {
        chowdsp::PoolAllocator<32, 8> allocator { 4 };
        REQUIRE (allocator.backing_buffer.size() == 128);
    }

    SECTION ("Allocate/Free")
    {
        chowdsp::PoolAllocator<32, 8> allocator { 4 };
        std::array<void*, 6> pointers {};
        for (size_t i = 0; i < 5; ++i)
        {
            pointers[i] = allocator.allocate_chunk();
            if (i < 4)
                REQUIRE (pointers[i] != nullptr);
            else
                REQUIRE (pointers[i] == nullptr);
        }
        pointers.back() = &allocator;
        for (auto* pointer : pointers)
            allocator.free_chunk (pointer);
    }

    SECTION ("Allocate/Free Object")
    {
        chowdsp::PoolAllocator<32, 8> allocator { 4 };

        struct Thing
        {
            int64_t x {};
            int64_t y {};
            int64_t z {};
        };
        auto* thing = allocator.allocate<Thing> (0, 1, 2);
        REQUIRE (thing != nullptr);
        REQUIRE (thing->x == 0);
        REQUIRE (thing->y == 1);
        REQUIRE (thing->z == 2);
        allocator.free (thing);

        allocator.free<Thing> (nullptr);

        Thing xx {};
        allocator.free<Thing> (&xx);
    }

    SECTION ("Object Pool Test")
    {
        struct Thing
        {
            int64_t x {};
            int64_t y {};
            int64_t z {};
        };

        chowdsp::ObjectPool<Thing> allocator { 4 };

        auto* thing = allocator.allocate (0, 1, 2);
        REQUIRE (thing != nullptr);
        REQUIRE (thing->x == 0);
        REQUIRE (thing->y == 1);
        REQUIRE (thing->z == 2);
        allocator.free (thing);

        Thing xx {};
        allocator.free (&xx);
    }
}
