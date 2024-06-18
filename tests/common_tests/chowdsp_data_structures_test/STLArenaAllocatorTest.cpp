#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

TEST_CASE ("STL Arena Allocator Test", "[common][data-structures]")
{
    using Arena = chowdsp::ArenaAllocator<>;
    Arena arena { 512 };

    using Alloc = chowdsp::STLArenaAllocator<int, Arena>;
    Alloc alloc { arena };

    using custom_vector = std::vector<int, Alloc>;
    custom_vector vec { { 1, 2, 3, 4 }, alloc };
    REQUIRE (vec.size() == 4);
    REQUIRE (vec.front() == 1);
    REQUIRE (vec.back() == 4);

    vec.push_back (5);
    REQUIRE (vec.size() == 5);
    REQUIRE (vec.back() == 5);

    vec.erase (vec.begin());
    REQUIRE (vec.size() == 4);
    vec.insert (vec.begin(), 0);
    REQUIRE (vec.size() == 5);
    REQUIRE (vec.front() == 0);
}
